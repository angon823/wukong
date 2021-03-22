#ifndef WUKONG_ASIO_SERVER_H
#define WUKONG_ASIO_SERVER_H

#include "define.h"
#include <csignal>
#include "timer/time_manager.h"

namespace wukong {
class ILogicServer;

namespace net
{
class IpAddress;

// server enumeration
enum class NetServerType
{
    TCP,
    UDP,
    KCP,
};

class AsioNetServer
{
public:

    explicit AsioNetServer(ILogicServer* server)
        : server_(server)
    {
        timerManager_.Init();
    }
    virtual ~AsioNetServer() = default;

    virtual NetServerType GetType() const = 0;
    // 初始化
    virtual bool Init(const IpAddress& addr, int32_t thread_num) = 0;
    // 开启服务, 将进入循环
    virtual void Loop() = 0;
    // 关闭服务
    virtual void Exit() = 0;


    // 注册信号 FIXME 有点简陋
    static void SetSignal(int sig, void signal_handler(int sig))
    {
        signal(sig, signal_handler);
    }

    TimerManager& GetTimerManager() {return timerManager_; }

protected:
    ILogicServer* server_{nullptr};
    TimerManager timerManager_;
};


//class server
}
}
#endif //WUKONG_ASIO_SERVER_H
