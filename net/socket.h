#ifndef __Socket_H__
#define __Socket_H__

#include "ip_address.h"

namespace wukong {
namespace net {
	
class Socket
{
public:
	Socket() {}
	explicit Socket(int32_t fd);

	int32_t Getfd() { return fd_; }
	bool IsValid() { return fd_ >= 0; }

	int32_t SetReusePort();
	int32_t SetReuseAddr();
	int32_t SetTcpNoDelay();
	int32_t SetNoBlock();
	int32_t SetKeepAlive();

private:
	int32_t fd_;
};

Socket CreateSocket(int32_t fd, int32_t falgs);
Socket CreateSocket(int16_t family, int32_t falgs = 0x7fffffff);

} // namespace net
} // namespace wukong



#endif // __Socket_H__

