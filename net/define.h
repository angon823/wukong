#ifndef WUKONG_DEFINE_H
#define WUKONG_DEFINE_H

#include <memory>


namespace wukong
{
namespace net
{


#define NEW_OBJ(cls, ...) new NewObj<cls>(##__VA_ARGS__)
#define NEW_SHARED_PTR(ptr) MakeSPtr(ptr)
#define MAKE_SHARED_PTR(class_name, ...) NEW_SHARED_PTR(NEW_OBJ(class_name, ##__VA_ARGS__))

template <typename  T>
inline std::shared_ptr<T> MakeSPtr(T* ptr)
{
    return std::shared_ptr<T>(ptr);
}

template <typename T, typename  ...Args>
inline T* NewObj(Args&&... args)
{
    return new T(std::forward<Args>(args)...);
}


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


constexpr int32_t kPollTimeoutMs = 5; //ms
}
}

#endif //WUKONG_DEFINE_H
