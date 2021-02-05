#include "listener.h"

using namespace wukong::net;

Listener::Listener(NewConnectionCallBack cb)
	: newConnectionCallBack_(cb)
{
}

int32_t Listener::Listen(const IpAddress & addr)
{
	Socket sock = CreateSocket(addr.Family());
	if (!sock.IsValid())
	{
		return -1;
	}

	acceptor_.SetSocket(sock);
	int32_t ret = acceptor_.Bind(addr);
	if (ret < 0)
	{
	}


	ret = acceptor_.Listen();
	if (ret < 0)
	{
		
	}

	return ret;
}

int32_t Listener::handleReadEvent(int32_t event)
{
	// read
	if (event & 1)
	{
		IpAddress client_addr;
		auto sock = acceptor_.Accept(&client_addr);
		if (!sock.IsValid())
		{
			return -1;
		}

		if (newConnectionCallBack_) {
			newConnectionCallBack_(sock);
		}
	}
}

