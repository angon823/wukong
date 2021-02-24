#ifndef WUKONG_THREAD_LOOP_H
#define WUKONG_THREAD_LOOP_H

#include "define.h"
#include "poller/poller.h"
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include "mq.h"
#include "message_procceor.h"
#include "i_net_server.h"

namespace wukong
{
namespace net
{
class PollEvent;
class INetServer;
class ThreadLoop
{
public:
    ThreadLoop() = default;
    int32_t Start(NetServerType type, int32_t timeout);
    void Stop();

    template<typename ...Args>
    inline void PushMessage(Args&& ... args)
    {
        in_msg_mq_.PushBack(std::make_shared<Message>(std::forward<Args>(args)...));
    }
    void PopMessage(std::vector<MessageSPtr>& out, uint32_t max_count);

    void UpdateWritable(int32_t fd, bool enable);

private:
    void loop(int32_t timeout);
    void processMsg();
    int32_t poll(int32_t timeout);

    template<typename ...Args>
    inline void dispatchMessage(Args&& ... args)
    {
        out_msg_mq_.PushBack(std::make_shared<Message>(std::forward<Args>(args)...));
    }

    void handleEvent(const PollEvent& event);
    void handleErr(int32_t fd);
    void handleRead(int32_t fd);
    void handleWrite(int32_t fd);

    void registerMessage();
    void onMessageNewCon(const MessageSPtr& msg);

    bool addConn(const ConnectionSPtr& con);
    ConnectionSPtr getConn(int32_t fd);
    void removeConn(int32_t fd);
    void closeFd(int32_t fd);

private:
    std::atomic<bool> quit_{false};
    std::unique_ptr<std::thread> thread_;
    std::unique_ptr<Poller> poller_{ nullptr };
    std::unordered_map<int32_t, ConnectionSPtr> connections_;

    MQ<MessageSPtr> in_msg_mq_; // net server -> thread loop
    MQ<MessageSPtr> out_msg_mq_; // thread loop -> net server
    MessageProcessor msg_processor_;
};

}
}
#endif //WUKONG_THREAD_LOOP_H
