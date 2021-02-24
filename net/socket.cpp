#include "socket.h"

#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#else
#include <netinet/tcp.h>
#include <fcntl.h>
#endif

using namespace wukong::net;

Socket::Socket(int32_t fd)
	:fd_(fd)
{
}

int32_t Socket::SetTcpNoDelay()
{
#if defined(SO_REUSEADDR) && !defined(_WIN32)
	int optval = 1;
	::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
		(void*)&optval, static_cast<socklen_t>(sizeof optval));
#else
#endif
	return 0;
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

int32_t Socket::SetKeepAlive()
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

int32_t Socket::MakeFdFine(int32_t flags)
{
    auto ret = SetNoBlock();
    if (ret < 0)
    {
        return ret;
    }
    ret = SetTcpNoDelay();
    if (ret < 0)
    {
        return ret;
    }
    ret = SetKeepAlive();
    if (ret < 0)
    {
        return ret;
    }
    ret = SetReuseAddr();
    if (ret < 0)
    {
        return ret;
    }

    if (flags & 1)
    {
        ret = SetReusePort();
        if (ret < 0)
        {
            return ret;
        }
    }
    return 0;
}

void Socket::Close()
{
#ifdef _WIN32
    closesocket(fd_);
#else
    close(fd_);
#endif
    fd_ = -1;
}

Socket wukong::net::CreateSocket(int32_t fd, int32_t flags)
{
	Socket sock(fd);
	if (fd < 0)
	{
		return sock;
	}

    if (sock.MakeFdFine(flags) != 0)
    {
        return Socket(-1);
    }

	return sock;
}

Socket wukong::net::CreateSocket(int32_t flags)
{
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif
	int32_t fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        return Socket(-1);
    }

	Socket sock(fd);
    if (sock.MakeFdFine(flags) != 0)
    {
        return Socket(-1);
    }
    return sock;
}