#ifndef __H_EPOLL_H__
#define __H_EPOLL_H__

#include <cstdint>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>
#include "poller.h"

namespace wukong{
namespace net{

class Epoll : public Poller
{
public:
    Epoll();
    ~Epoll();

    int32_t Init() override;
    int32_t CtlFd(int32_t fd, int32_t op, uint32_t events) override;
    int32_t Poll(int32_t timeout, std::vector<PollEvent>& activeEvents) override;

    int32_t epoll_init();
    int32_t epoll_wait(int32_t timeout = -1);
    int32_t epoll_add(int32_t, uint32_t);
    int32_t epoll_del(int32_t);
    int32_t epoll_mod(int32_t, uint32_t events);
    struct epoll_event* get_event(int32_t);
    void epoll_destroy();

private:
    int32_t size_;
    int32_t epoll_fd_;
    epoll_event event_;
    epoll_event* epollEvents_;
};

}
}
#endif

