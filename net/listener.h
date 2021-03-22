#ifndef __H_Listener_H__
#define __H_Listener_H__

#include "socket.h"
#include <memory>
#include <vector>
#include "poller/poll_event.h"
#include "asio_server.h"

namespace wukong {
namespace net {

class Poller;
class AsioNetServer;

class Listener
{
public:
    using EventCallBack = std::function<void(int32_t, int32_t)>;

    explicit  Listener(EventCallBack cb, NetServerType typ);
	int32_t Listen(const IpAddress & addr);
    Socket Accept(IpAddress *addr);
	int32_t Poll();

    void Close();
    int32_t GetFd() const { return sock_.GetFd(); }

private:
    Socket sock_{};
    std::unique_ptr<Poller> poller_{nullptr};
    std::vector<PollEvent> activeEvents_;
    NetServerType server_type_;
    EventCallBack event_cb_;
};

}
}

#endif


