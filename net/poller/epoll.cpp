#include <cstring>
#include <cassert>
#include "epoll.h"
#include "poll_event.h"

namespace wukong{
namespace net{


Epoll::~Epoll()
{
    epoll_destroy();
}

int32_t Epoll::Init()
{
    return epoll_init();
}

void Epoll::Uninit()
{
    epoll_destroy();
}

int32_t Epoll::CtlFd(int32_t fd, int32_t op, uint32_t events)
{
    if (op == kPollAdd)
    {
        return epoll_add(fd, events);
    }
    if (op == kPollDel)
    {
        return epoll_del(fd);
    }
    if (op == kPollMod)
    {
        return epoll_mod(fd, events);
    }
    return -1;
}

int32_t Epoll::Poll(int32_t timeout, std::vector<PollEvent>& activeEvents)
{
    int32_t eventNum = epoll_wait(timeout);
    for (int i = 0; i < eventNum; ++i)
    {
        auto *event = get_event(i);
        activeEvents.emplace_back(int32_t(event->data.fd), uint32_t(event->events));
    }
    return eventNum;
}

int32_t Epoll::epoll_init()
{
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        return -1;
    }


    memset(&event_, 0, sizeof(event_));

    size_ = 65535;
    epollEvents_ = new epoll_event[size_];
    if (nullptr == epollEvents_) {
        return -1;
    }
    return 0;
}

int32_t Epoll::epoll_wait(int32_t timeout)
{
    return ::epoll_wait(epoll_fd_, epollEvents_, size_, timeout);
}

int32_t Epoll::epoll_add(int32_t fd, uint32_t events)
{
    if (fd < 0)
    {
        return  -1;
    }

    event_.data.fd = fd;
    event_.events = events;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event_) != 0)
    {
        return -1;
    }

    return 0;
}

int32_t Epoll::epoll_del(int32_t fd)
{
    if (fd < 0)
    {
        return  -1;
    }
    event_.data.fd = fd;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event_);
}

void Epoll::epoll_destroy()
{
    delete[] epollEvents_;
    close(epoll_fd_);
}

epoll_event* Epoll::get_event(int32_t idx)
{
    return &epollEvents_[idx];
}

int32_t Epoll::epoll_mod(int32_t fd, uint32_t events)
{
    if (fd < 0)
    {
        return -1;
    }
    event_.events = events;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event_);
}

}
}