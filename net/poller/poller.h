#ifndef __H_POLLER_H__
#define __H_POLLER_H__
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include "poll_event.h"
#include "util/noncopyable.h"

#ifdef __linux__
#include <sys/epoll.h>
#endif

namespace wukong
{
namespace net
{

// operation enumeration
enum
{
    kPollAdd,
    kPollDel,
    kPollMod,
};

// event enumeration
enum
{
#ifdef __linux__
    kReadEvent = EPOLLIN | EPOLLPRI,
    kWriteEvent = EPOLLOUT,
    kErrEvent = EPOLLERR | EPOLLHUP,
    kDefaultEpollEvent = kReadEvent | kErrEvent | EPOLLET,

    kDefaultPollEvent = kDefaultEpollEvent,

#elif _WIN32
    // todo
    kDefaultPollEvent = 0;
#endif
};

class Poller : public noncopyable
{
public:
    virtual ~Poller() = default;
    virtual int32_t Init() = 0;
    virtual int32_t CtlFd(int32_t fd, int32_t op, uint32_t events) = 0;
    virtual int32_t Poll(int32_t timeout, std::vector<PollEvent>& activeEvents) = 0;

protected:
};


}
}
#endif
