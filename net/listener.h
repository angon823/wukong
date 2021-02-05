#ifndef __H_Listener_H__
#define __H_Listener_H__

#include "acceptor.h"
#include <functional>


namespace wukong {
namespace net {

using NewConnectionCallBack = std::function<void(Socket)>;

class Listener
{
public:
	Listener(NewConnectionCallBack cb);

	int32_t Listen(const IpAddress & addr);

	int32_t handleReadEvent(int32_t event);

private:
	NewConnectionCallBack newConnectionCallBack_;
	Acceptor acceptor_;
};

}
}

#endif


