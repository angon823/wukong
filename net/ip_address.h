#ifndef __Ip_Address_H__
#define __Ip_Address_H__

#include <string>
#include <inttypes.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <winerror.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#endif

namespace wukong{
namespace net {


class IpAddress
{
public:
	IpAddress() {}
	IpAddress(uint16_t port)
	{
		addr_.sin_family = AF_INET;
		addr_.sin_addr.s_addr = INADDR_LOOPBACK;
		addr_.sin_port = htons(port);
	}

	IpAddress(const std::string& ip, uint16_t port)
	{
		addr_.sin_family = AF_INET;
		addr_.sin_addr.s_addr = inet_addr(ip.c_str());
		addr_.sin_port = htons(port);
	}

	int16_t Family() const 	{ return addr_.sin_family; }
	uint16_t Port() const { return addr_.sin_port; }
	std::string Ip() const { return inet_ntoa(addr_.sin_addr); }
	sockaddr* SockAddr() const { return (sockaddr*)&addr_;	}

private:
	struct sockaddr_in addr_;
};

}
}

#endif
