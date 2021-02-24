#ifndef __H_Listener_H__
#define __H_Listener_H__

#include "socket.h"
#include <memory>
#include <vector>
#include "poller/poll_event.h"

namespace wukong {
namespace net {

class Poller;
class INetServer;

class Listener
{
public:
    explicit  Listener(INetServer*);
	int32_t Listen(const IpAddress & addr);
	int32_t Poll(int32_t timeout);
    Socket Accept(IpAddress *addr);

    void    Close()  { sock_.Close(); }
    int32_t GetFd() const { return sock_.GetFd(); }

private:
    Socket sock_{};
    std::unique_ptr<Poller> poller_{nullptr};
    std::vector<PollEvent> activeEvents_;
    INetServer* i_net_server_;
};

}
}

#endif


