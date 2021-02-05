#include "acceptor.h"

using namespace wukong::net;


void wukong::net::Acceptor::SetSocket(Socket sock)
{
	sock_ = sock;
}

int32_t Acceptor::Bind(const IpAddress & addr)
{
	int ret = ::bind(sock_.Getfd(), addr.SockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
	if (ret < 0)
	{
	}
	return ret;
}


int32_t Acceptor::Listen()
{
	int ret = ::listen(sock_.Getfd(), SOMAXCONN);
	if (ret < 0)
	{
	}
	return ret;
}

Socket Acceptor::Accept(IpAddress *addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
	int32_t connfd = ::accept(sock_.Getfd(), addr->SockAddr(), &addrlen);
	if (connfd < 0)
	{

	}

	Socket sock(connfd);
	sock.SetNoBlock();
	sock.SetTcpNoDelay();
	sock.SetKeepAlive();

	return sock;
}