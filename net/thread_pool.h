#ifndef __H_THREAD_POOL_H__
#define __H_THREAD_POOL_H__

#include <vector>
#include <memory>
#include "define.h"

namespace wukong
{
namespace net
{
class AsioNetServer;
class ThreadLoop;
class ThreadPool
{
public:
    using ThreadLoopList = std::vector<ThreadLoopSPtr>;

    int32_t Create(AsioNetServer* i_server, int32_t thread_num);
    int32_t Stop();
    const ThreadLoopSPtr& GetLoop(size_t index);
    const ThreadLoopSPtr& GetNextLoop();
    ThreadLoopList::const_iterator begin() {return threads_.begin();}
    ThreadLoopList::const_iterator end() {return threads_.end();}

private:
    int32_t next_loop_index_{0};
    ThreadLoopList threads_;
};
}
}

#endif

