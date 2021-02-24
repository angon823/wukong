#include <functional>
#include <cassert>
#include "tcp_server.h"
#include "connection.h"
#include "../base/log.h"
#include "thread_pool.h"
#include "thread_loop.h"
#include "logic_server_interface.h"

using namespace wukong::net;


TcpServer::TcpServer(wukong::ILogicServer *logic_server)
    : INetServer(logic_server)
{
    assert(server_);
}

int32_t TcpServer::Init(const IpAddress &addr, int32_t thread_num)
{
    assert(0 <= thread_num);

    listener_ = std::unique_ptr<Listener>(new Listener(this));
    int32_t ret = listener_->Listen(addr);
    if (ret != 0)
    {
        return ret;
    }

    thread_pool_ = std::unique_ptr<ThreadPool>(new ThreadPool());
    if (thread_pool_->Create(this, thread_num, poll_timeout_ms_) != 0)
    {
        return -1;
    }

    registerMessage();

    LogInfo("tcpserver start!");
    return ret;
}

int32_t TcpServer::Serve()
{
    while (!quit_)
    {
        handleSignal();
        processMsg();
        handleTimer();
        poll();
    }
	return 0;
}

void TcpServer::Shutdown()
{
    quit_ = true;
	listener_->Close();
    thread_pool_->Stop();
    connections_.clear();
}

bool TcpServer::CloseConnection(const ConnectionSPtr& con)
{
    if (removeConnection(con->GetFd()))
    {
        con->SetState(kConnectionStateDisconnectByServer);
        con->GetThreadLoop()->PushMessage(kDownCloseCon, con->GetFd());
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
    listener_->Poll(poll_timeout_ms_);
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

}

void TcpServer::handleSignal()
{

}

void TcpServer::HandleListenErr(int32_t fd)
{
    int32_t errcode = 0;
    socklen_t len = (socklen_t)sizeof(errcode);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &errcode, &len);

    LogError("TcpServer error:%d Shutdown...", errcode);
    Shutdown();
}

void TcpServer::HandleListenRead(int32_t fd)
{
    // ET mode should accept all
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
            server_->OnMessageRecv(con);
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
    loop->PushMessage(MessageOnNewConAccept(sock.GetFd(), con));
}

void TcpServer::onNewConnectionInitFinish(int32_t fd, int32_t err)
{
    auto con = GetConnection(fd);
    assert(con);
    if (err == 0)
    {
        con->SetState(kConnectionStateConnect);
        server_->ConnectionCallBack(con);
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
        server_->ConnectionCallBack(con);
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
        server_->ConnectionCallBack(con);
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

TcpServerSPtr wukong::net::CreateAndServeTcpServer(const IpAddress & addr)
{
	auto tcp_server = std::make_shared<TcpServer>(new defaultServer()) ;
    int32_t err = tcp_server->Init(addr);
    if (err != 0)
    {
        return nullptr;
    }

	err = tcp_server->Serve();
	if (err != 0)
	{
		return nullptr;
	}



	return tcp_server;
}
