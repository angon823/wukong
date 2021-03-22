#include "thread_loop.h"
#include "connection.h"
#include "log/log.h"
#include "poller/epoll.h"
#include "poller/poller_factory.h"
#include <cassert>
#include "define.h"

using namespace wukong::net;


int32_t ThreadLoop::Start(NetServerType type)
{
    poller_ = std::unique_ptr<Poller>(CreatePoller(type));
    if (poller_ == nullptr)
    {
        return -1;
    }

    if (poller_->Init() != 0)
    {
        return -1;
    }

    registerMessage();

    thread_ = std::unique_ptr<std::thread>(new std::thread([this] { loop(); }));
    return 0;
}

void ThreadLoop::Stop()
{
    quit_ = true;
    poller_->Uninit();
    for (const auto& con : connections_)
    {
        con.second->Close();
    }
    thread_->join();
}

void ThreadLoop::PopMessage(std::vector<MessageSPtr> &out, uint32_t max_count)
{
    for (uint32_t i = 0; i < max_count; ++i)
    {
        auto msg = out_msg_mq_.PopFront();
        if (msg == nullptr)
        {
            break;
        }
        out.emplace_back(msg);
    }
}

void ThreadLoop::UpdateWritable(int32_t fd, bool enable)
{
    uint32_t events = kDefaultPollEvent;
    if (enable)
    {
        events |= kWriteEvent;
    }
    poller_->CtlFd(fd, kPollMod, events);
}

void ThreadLoop::loop()
{
    LogInfo("thread loop:%p, start", this);
    while (!quit_)
    {
        poll();
        processMsg();
    }
    LogInfo("thread loop:%p, exit", this);
}

int32_t ThreadLoop::poll()
{
    static std::vector<PollEvent> activeEvents;
    activeEvents.clear();
    if (poller_->Poll(kPollTimeoutMs, activeEvents) < 0)
    {
        LogFatal("loop:%p Poll failed!", this);
        // FIXME
        return  -1;
    }

    if (activeEvents.empty()) return 0;

    for (const auto& event : activeEvents)
    {
        handleEvent(event);
    }

    return activeEvents.size();
}

void ThreadLoop::handleEvent(const PollEvent& event)
{
    auto events = event.Events();
    auto fd = event.Fd();
    if (events & (EPOLLERR |EPOLLHUP))
    {
        handleErr(fd);
    }
    else if (events & kReadEvent)
    {
        handleRead(fd);
    }
    else if (events & kWriteEvent)
    {
        handleWrite(fd);
    }
}

void ThreadLoop::handleErr(int32_t fd)
{
    int32_t errcode = 0;
    socklen_t len = (socklen_t)sizeof(errcode);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &errcode, &len);

    auto con = getConn(fd);
    if (con)
    {
        con->SetState(kConnectionStateBad);
    }
    closeFd(fd);

    dispatchMessageToServer(new MessageErr(kUpOnConErr, fd, errcode));
}

void ThreadLoop::handleRead(int32_t fd)
{
    auto con = getConn(fd);
    if (con)
    {
        auto ret = con->RecvMsg();
        if (ret == Success)
        {
            emplaceMessageToServer(kUpOnConMessage, fd);
        }
        else if (ret == ErrPeerConClosed)
        {
            con->SetState(kConnectionStateDisconnectByPeer);
            closeFd(fd);
            emplaceMessageToServer(kUpOnConDisconnect, fd);
        }
        else
        {
            handleErr(fd);
        }
    }
    else
    {
        LogError("can't find con by fd:%d, close it", fd);
        closeFd(fd);
    }
}

void ThreadLoop::handleWrite(int32_t fd)
{
    auto con = getConn(fd);
    if (con)
    {
        auto n = con->HandleWritable();
        if (n < 0)
        {
            handleErr(fd);
        }
        else if (n == 0)
        {
            UpdateWritable(fd, false);
        }
    }
    else
    {
        LogError("handleWrite fd:%d buf can't find con", fd);
        closeFd(fd);
    }
}

void ThreadLoop::registerMessage()
{
    msg_processor_.RegisterHandler(kDownNewConAccept, [this](const MessageSPtr& msg)
    {
        onMessageNewCon(msg);
    });


    msg_processor_.RegisterHandler(kDownCloseCon, [this](const MessageSPtr& msg)
    {
        closeFd(msg->fd_);
    });

    msg_processor_.RegisterHandler(kDownEnableWrite, [this](const MessageSPtr& msg)
    {
        UpdateWritable(msg->fd_, true);
    });
}

void ThreadLoop::processMsg()
{
    MessageSPtr msg = nullptr;
    while ((msg = in_msg_mq_.PopFront()))
    {
        msg_processor_.Process(msg);
    }
}

void ThreadLoop::onMessageNewCon(const MessageSPtr &msg)
{
    auto *new_con_msg = dynamic_cast<MessageOnNewConAccept *>(msg.get());
    assert(new_con_msg);
    int32_t errcode = 0;
    if (!addConn(new_con_msg->con_))
    {
        errcode = -1;
        goto end;
    }

    if (poller_->CtlFd(msg->fd_, kPollAdd, kDefaultPollEvent) != 0)
    {
        removeConn(msg->fd_);
        errcode = -2;
        goto end;
    }

    end:
    dispatchMessageToServer(NewObj<MessageNewConInitFinish>(msg->fd_, errcode));
}

void ThreadLoop::closeFd(int32_t fd)
{
    if (poller_->CtlFd(fd, kPollDel, 0) != 0)
    {
        LogWarn("close fd:%d by remove from poll failed", fd);
    }

    // FIXME 放在主线程里？
    auto con = getConn(fd);
    if (con)
    {
        con->Close();
    }
    removeConn(fd);
}

bool ThreadLoop::addConn(const ConnectionSPtr &con)
{
    if (connections_.count(con->GetFd()) > 0)
    {
        LogError("con:%s already in loop", con->ToString().c_str());
        assert(true);
        return false;
    }

    connections_[con->GetFd()] = con;

    return true;
}

ConnectionSPtr ThreadLoop::getConn(int32_t fd)
{
    auto it =  connections_.find(fd);
    if (it != connections_.end())
    {
        return it->second;
    }
    return nullptr;
}

void ThreadLoop::removeConn(int32_t fd)
{
    connections_.erase(fd);
}



