#ifndef WUKONG_DEFINE_H
#define WUKONG_DEFINE_H

#include <memory>


namespace wukong
{
namespace net
{

class Connection;
using ConnectionSPtr = std::shared_ptr<Connection>;
using HighWaterMarkCallBack = std::function<void(const ConnectionSPtr&, uint32_t)>;

class TcpServer;
using TcpServerSPtr = std::shared_ptr<TcpServer>;

class ThreadLoop;
using ThreadLoopSPtr = std::shared_ptr<ThreadLoop>;

class Message;
using MessageSPtr = std::shared_ptr<Message>;



///========== ENUM =========///



// error code

enum ErrCode
{
    Success,
    ErrPeerConClosed,
    ErrBufferNotEnough,
    ErrSocketErr,


};


}
}

#endif //WUKONG_DEFINE_H
