#ifndef WUKONG_THREAD_LOOP_H
#define WUKONG_THREAD_LOOP_H

#include "define.h"
#include "poller/poller.h"
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include "util/mq.h"
#include "message_procceor.h"
#include "asio_server.h"

namespace wukong
{
namespace net
{
class PollEvent;
class AsioNetServer;
class ThreadLoop
{
public:
    ThreadLoop() = default;
    int32_t Start(NetServerType type);
    void Stop();

    template<typename ...Args>
    inline void EmplaceMessage(Args&& ... args)
    {
        in_msg_mq_.PushBack(std::make_shared<Message>(std::forward<Args>(args)...));
    }
    inline void PushMessage(Message* ptr)
    {
        in_msg_mq_.PushBack(std::shared_ptr<Message>(ptr));
    }
    void PopMessage(std::vector<MessageSPtr>& out, uint32_t max_count);

    void UpdateWritable(int32_t fd, bool enable);

private:
    void loop();
    void processMsg();
    int32_t poll();

    template<typename ...Args>
    inline void emplaceMessageToServer(Args&& ... args)
    {
        out_msg_mq_.PushBack(std::make_shared<Message>(std::forward<Args>(args)...));
    }
    inline void dispatchMessageToServer(Message* msg)
    {
        out_msg_mq_.PushBack(std::shared_ptr<Message>(msg));
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

    ReaderWriterMQ<MessageSPtr> in_msg_mq_; // net server -> thread loop
    ReaderWriterMQ<MessageSPtr> out_msg_mq_; // thread loop -> net server
    MessageProcessor msg_processor_;
};

}
}
#endif //WUKONG_THREAD_LOOP_H
