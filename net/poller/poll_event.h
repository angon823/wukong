#ifndef __H_EVENT_H__
#define __H_EVENT_H__
#include <cstdint>

namespace wukong
{
namespace net
{

class PollEvent
{
public:
    PollEvent() = default;
    PollEvent(int32_t fd, uint32_t events)
            : fd_(fd)
            , events_(events)
    {}

    inline uint32_t Events() const { return events_; }
    inline int32_t Fd() const { return fd_; }

private:
    int32_t fd_;
    uint32_t events_;
};


} // net
} // wukong
#endif // __H_EVENT_H__

