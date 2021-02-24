#ifndef __H_Socket_H__
#define __H_Socket_H__

#include "ip_address.h"

namespace wukong {
namespace net {
	
class Socket
{
public:
	Socket() = default;
	explicit Socket(int32_t fd);

	int32_t GetFd() const { return fd_; }
	bool IsValid() const { return fd_ >= 0; }

	int32_t MakeFdFine(int32_t flags);
	void Close();

	int32_t SetReusePort();
	int32_t SetReuseAddr();
	int32_t SetTcpNoDelay();
	int32_t SetNoBlock();
	int32_t SetKeepAlive();

private:
	int32_t fd_;
};

Socket CreateSocket(int32_t fd, int32_t flags);
Socket CreateSocket(int32_t flags = 0x7fffffff);
} // namespace net
} // namespace wukong


#endif // __H_Socket_H__

