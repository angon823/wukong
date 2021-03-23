#include "listener.h"
#include <cstdio>
#include "poller/epoll.h"
#include "poller/poller_factory.h"
#include <cassert>
using namespace wukong::net;

Listener::Listener(EventCallBack  cb, NetServerType typ)
    : server_type_(typ)
    , event_cb_(std::move(cb))
{
    assert(event_cb_);
}

int32_t Listener::Listen(const IpAddress & addr)
{
    sock_ = CreateSocket(0);
	if (!sock_.IsValid())
	{
		return -1;
	}

    int32_t ret = ::bind(sock_.GetFd(), addr.SockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if (ret < 0)
    {
        perror("");
    }

    ret = ::listen(sock_.GetFd(), SOMAXCONN);
    if (ret < 0)
    {
        perror("");
        return ret;
    }

    poller_ = std::unique_ptr<Poller>(CreatePoller(server_type_));
    if (poller_ == nullptr)
    {
        return -1;
    }
    ret = poller_->Init();
    if (ret != 0)
    {
        return ret;
    }

    ret = poller_->CtlFd(sock_.GetFd(), kPollAdd, kDefaultPollEvent);
    if (ret != 0)
    {
        return ret;
    }

    return ret;
}


Socket Listener::Accept(IpAddress *addr)
{
    auto addrLen = static_cast<socklen_t>(sizeof *addr);
    int32_t connFd = ::accept(sock_.GetFd(), (sockaddr*)addr->SockAddr(), &addrLen);
    if (connFd < 0)
    {
        if (errno != EAGAIN)
            perror("accept error!");
        return Socket(-1);
    }

    Socket sock(connFd);
    sock.SetNoBlock();
    sock.SetTcpNoDelay();
    sock.SetKeepAlive();

    return sock;
}

int32_t Listener::Poll()
{
    activeEvents_.clear();
    int32_t num = poller_->Poll(kPollTimeoutMs, activeEvents_);
    if (num <= 0)
    {
        return num;
    }

    for (const auto &event : activeEvents_)
    {
        uint32_t events = event.Events();
        int32_t sockFd = event.Fd();
        event_cb_(sockFd, events);
    }
    return num;
}

void Listener::Close()
{
    sock_.Close();
    if (poller_)
    {
        poller_->Uninit();
    }
}


