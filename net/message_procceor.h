#ifndef WUKONG_MESSAGE_PROCESSOR_H
#define WUKONG_MESSAGE_PROCESSOR_H
#include "message.h"
#include <memory>
#include <map>
#include <functional>

namespace wukong{
namespace net
{

class MessageProcessor
{
public:
    using MessageHandleFunctor = std::function<void(const MessageSPtr&)>;

    bool RegisterHandler(MessageType type, const MessageHandleFunctor& functor)
    {
        return handlers_.emplace(type, functor).second;
    }

    void Process(const MessageSPtr& msg)
    {
        auto it = handlers_.find(msg->typ_);
        if (it == handlers_.end())
        {
            return;
        }
        it->second(msg);
    }

private:
    std::map<MessageType, MessageHandleFunctor> handlers_{};
};


}
}
#endif //WUKONG_MESSAGE_PROCCEOR_H
