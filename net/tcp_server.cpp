#include <functional>
#include <cassert>
#include "tcp_server.h"
#include "connection.h"
#include "log/log.h"
#include "thread_pool.h"
#include "thread_loop.h"
#include "logic_server_interface.h"

using namespace wukong::net;


TcpServer::TcpServer(wukong::ILogicServer *logic_server)
    : AsioNetServer(logic_server)
{
    assert(logic_server_);
}


TcpServer::~TcpServer()
{
    if (listener_)
    {
        listener_->Close();
    }
    if (thread_pool_)
    {
        thread_pool_->Stop();
    }
    connections_.clear();
    LogInfo("tcpserver exit!");
}


bool TcpServer::Init(const IpAddress &addr, int32_t thread_num)
{
    assert(0 <= thread_num);

    listener_ = std::unique_ptr<Listener>(new Listener(std::bind(&TcpServer::handleListenEvent, this, std::placeholders::_1, std::placeholders::_2), NetServerType::TCP));
    if (listener_->Listen(addr) != 0)
    {
        return false;
    }

    // FIXME 线程池的创建时异步的， Loop调用实际应该在线程创建完成之后开始
    thread_pool_ = std::unique_ptr<ThreadPool>(new ThreadPool());
    if (thread_pool_->Create(this, thread_num) != 0)
    {
        return false;
    }

    registerMessage();

    LogInfo("tcpserver start!");
    return true;
}

void TcpServer::Loop()
{
    while (!quit_)
    {
        poll();
        processMsg();
        handleTimer();
    }
}

void TcpServer::Exit()
{
    quit_ = true;
    LogInfo("tcpserver ready exit!");
}

bool TcpServer::CloseConnection(const ConnectionSPtr& con)
{
    if (removeConnection(con->GetFd()))
    {
        con->SetState(kConnectionStateDisconnectByServer);
        con->GetThreadLoop()->EmplaceMessage(kDownCloseCon, con->GetFd());
        return true;
    }
    return false;
}

ConnectionSPtr TcpServer::GetConnection(int32_t fd)
{
    auto it = connections_.find(fd);
    if (it == connections_.end())
    {
        return nullptr;
    }
    return it->second;
}

void TcpServer::poll()
{
    listener_->Poll();
}

void TcpServer::processMsg()
{
    static std::vector<MessageSPtr> messages;
    for (const auto& loop : *thread_pool_)
    {
        messages.clear();
        loop->PopMessage(messages, -1);
        if (messages.empty())
        {
            continue;
        }

        for (const auto& msg : messages)
        {
            msg_processor_.Process(msg);
        }
    }
}

void TcpServer::handleTimer()
{
    timer_manager_.Update(1);
}

void TcpServer::handleListenEvent(int32_t fd, uint32_t events)
{
    if (events & (EPOLLERR | EPOLLHUP))
    {
        handleListenErr(fd);
    }
    else if (events & (EPOLLIN | EPOLLPRI))
    {
        handleListenRead(fd);
    }
}

void TcpServer::handleListenErr(int32_t fd)
{
    int32_t errcode = 0;
    auto len = (socklen_t)sizeof(errcode);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &errcode, &len);

    LogError("TcpServer error:%d Exit...", errcode);
    Exit();
}

void TcpServer::handleListenRead(int32_t fd)
{
    // ET mode 多个accept只会触发一次read
    do
    {
        IpAddress addr;
        auto sock = listener_->Accept(&addr);
        if (!sock.IsValid())
        {
            break;
        }

        onNewConnectionAccept(sock, addr);
    } while (true);
}

void TcpServer::registerMessage()
{
    msg_processor_.RegisterHandler(kUpOnNewConInitFinish, [this](const MessageSPtr& msg)
    {
        auto* init_msg = dynamic_cast<MessageNewConInitFinish*>(msg.get());
        assert(init_msg);
        onNewConnectionInitFinish(msg->fd_, init_msg->errcode_);
    });

    msg_processor_.RegisterHandler(kUpOnConDisconnect, [this](const MessageSPtr& msg)
    {
        onConnectionDisconnect(msg->fd_);
    });

    msg_processor_.RegisterHandler(kUpOnConErr, [this](const MessageSPtr& msg)
    {
        auto* err_msg = dynamic_cast<MessageErr*>(msg.get());
        assert(err_msg);
        onConnectionBad(msg->fd_, err_msg->errcode_);
    });

    msg_processor_.RegisterHandler(kUpOnConMessage, [this](const MessageSPtr& msg)
    {
        auto con = GetConnection(msg->fd_);
        if (con)
        {
            logic_server_->OnMessageRecv(con);
        }
        assert(con); // todo should be happen
    });
}

void TcpServer::onNewConnectionAccept(const Socket& sock, const IpAddress& addr)
{
    if (!sock.IsValid())
    {
        return;
    }

    auto con = std::make_shared<Connection>(sock, addr);
    con->SetState(kConnectionStateConnecting);
    if (!addConnection(con))
    {
        return;
    }

    auto loop = thread_pool_->GetNextLoop();
    assert(loop);
    con->SetThreadLoop(loop.get());
    loop->PushMessage(new MessageOnNewConAccept(sock.GetFd(), con));
}

void TcpServer::onNewConnectionInitFinish(int32_t fd, int32_t err)
{
    auto con = GetConnection(fd);
    assert(con);
    if (err == 0)
    {
        con->SetState(kConnectionStateConnect);
        logic_server_->ConnectionCallBack(con);
    }
    else
    {
        con->Close();
        removeConnection(fd);
        LogError("init con:%s failed! errcode:%d", con->ToString().c_str(), err);
    }
}

void TcpServer::onConnectionDisconnect(int32_t fd)
{
    auto con = GetConnection(fd);
    if (con)
    {
        logic_server_->ConnectionCallBack(con);
        removeConnection(fd);
    }
    else
    {
        LogError("fd:%d disconnect but can't find con", fd);
    }
}

void TcpServer::onConnectionBad(int32_t fd, int32_t err_code)
{
    auto con = GetConnection(fd);
    if (con)
    {
        LogError("con:%s bad. errcode:%d", con->ToString().c_str(), err_code);
        logic_server_->ConnectionCallBack(con);
        removeConnection(fd);
    }
    else
    {
        LogError("fd:%d bad. errcode:%d", fd, err_code);
    }
}

bool TcpServer::addConnection(const ConnectionSPtr& con)
{
    if (connections_.count(con->GetFd()) > 0)
    {
        return false;
    }
    connections_[con->GetFd()] = con;
    return true;
}

bool TcpServer::removeConnection(int32_t fd)
{
    return connections_.erase(fd) > 0;
}
