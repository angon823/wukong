#ifndef __Ip_Address_H__
#define __Ip_Address_H__

#include <string>
#include <cinttypes>
#include <cstdio>

#ifdef _WIN32
#include <winsock2.h>
#include <winerror.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#elif __linux__
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif // _WIN32

namespace wukong{
namespace net {

class IpAddress
{
public:
	IpAddress() = default;

	explicit IpAddress(uint16_t port)
	{
		addr_.sin_family = AF_INET;
		addr_.sin_addr.s_addr = INADDR_ANY;
		addr_.sin_port = htons(port);
	}

	IpAddress(const std::string& ip, uint16_t port)
	{
		addr_.sin_family = AF_INET;
		addr_.sin_addr.s_addr = inet_addr(ip.c_str());
		addr_.sin_port = htons(port);
	}

	int16_t Family() const 	{ return addr_.sin_family; }
	uint16_t Port() const { return ntohs(addr_.sin_port); }
	std::string Ip() const { return inet_ntoa(addr_.sin_addr); }
	const sockaddr* SockAddr() const { return (sockaddr*)&addr_;	}

	std::string ToString() const {return Ip().append(":").append(std::to_string(Port()));}

private:
	struct sockaddr_in addr_{};
};

}
}

#endif //__Ip_Address_H__
