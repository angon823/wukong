#include "socket.h"

#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

using namespace wukong::net;

Socket::Socket(int32_t fd)
	:fd_(fd)
{
}


int32_t Socket::SetTcpNoDelay()
{
#if defined(SO_REUSEADDR) && !defined(_WIN32)
	int optval = 1
	::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
		(void*)&optval, static_cast<socklen_t>(sizeof optval));
#else
	return 0;
#endif
}

int32_t Socket::SetNoBlock()
{
#ifdef _WIN32
	unsigned long nonblocking = 1;
	if (ioctlsocket(fd_, FIONBIO, &nonblocking) == SOCKET_ERROR) {
		return -1;
	}
#else
	int flags;
	if ((flags = fcntl(fd_, F_GETFL, NULL)) < 0) {
		return -1;
	}
	if (!(flags & O_NONBLOCK)) {
		if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
			return -1;
		}
	}
#endif
	return 0;
}

int32_t wukong::net::Socket::SetKeepAlive()
{
#if defined(SO_KEEPALIVE) && !defined(_WIN32)
	int optval = 1;
	return setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE,
		(void*)&optval, static_cast<socklen_t>(sizeof optval));
#else
	return 0;
#endif
}

int32_t Socket::SetReuseAddr()
{
#if defined(SO_REUSEADDR) && !defined(_WIN32)
	int one = 1;
	/* REUSEADDR on Unix means, "don't hang on to this address after the
	 * listener is closed."  On Windows, though, it means "don't keep other
	 * processes from binding to this address while we're using it. */
	return setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (void*)&one,
		(socklen_t)sizeof(one));
#else
	return 0;
#endif
}

int32_t Socket::SetReusePort()
{
#if defined __linux__ && defined(SO_REUSEPORT)
	int one = 1;
	/* REUSEPORT on Linux 3.9+ means, "Multiple servers (processes or
	 * threads) can bind to the same port if they each set the option. */
	return setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, (void*)&one,
		(socklen_t)sizeof(one));
#else
	return 0;
#endif
}

Socket wukong::net::CreateSocket(int32_t fd, int32_t falgs)
{
	Socket sock(fd);
	if (fd < 0)
	{
		return sock;
	}

	// todo
	if (falgs & 1)
	{
	}

	auto ret = sock.SetNoBlock();
	if (ret < 0)
	{
	}
	ret = sock.SetTcpNoDelay();
	if (ret < 0)
	{
	}
	ret = sock.SetKeepAlive();
	if (ret < 0)
	{
	}
	ret = sock.SetReuseAddr();
	if (ret < 0)
	{
	}
	ret = sock.SetReusePort();
	if (ret < 0)
	{
	}
	return sock;
}

Socket wukong::net::CreateSocket(int16_t family, int32_t falgs)
{
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif
	int32_t fd = ::socket(family, SOCK_STREAM, 0);
	Socket sock(fd);
	if (fd < 0)
	{
		return sock;
	}

	return wukong::net::CreateSocket(fd, falgs);
}
