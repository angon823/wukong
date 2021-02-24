#include "thread_pool.h"
#include "i_net_server.h"
#include <memory>
#include <thread>
#include "thread_loop.h"

using namespace wukong::net;

int32_t ThreadPool::Create(INetServer* i_server, int32_t thread_num, int32_t timeout)
{
    for (int32_t i = 0; i < thread_num; ++i )
    {
        auto thread_loop = std::make_shared<ThreadLoop>();
        if (thread_loop->Start(i_server->GetType(), timeout) < 0)
        {
            return -1;
        }
        threads_.push_back(thread_loop);
    }
    return 0;
}

int32_t ThreadPool::Stop()
{
    for (const auto& thread : threads_)
    {
        thread->Stop();
    }
    threads_.clear();
    return 0;
}

const ThreadLoopSPtr &ThreadPool::GetNextLoop()
{
    if (threads_.empty())
    {
        static ThreadLoopSPtr empty(nullptr);
        return empty;
    }

    auto index = (next_loop_index_++) % threads_.size();
    return threads_[index % threads_.size()] ;
}

const ThreadLoopSPtr &ThreadPool::GetLoop(size_t index)
{
    if (index >= threads_.size())
    {
        static ThreadLoopSPtr empty(nullptr);
        return empty;
    }

    return threads_[index];
}
