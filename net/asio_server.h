#ifndef WUKONG_ASIO_SERVER_H
#define WUKONG_ASIO_SERVER_H

#include "define.h"
#include <csignal>
#include "timer/time_manager.h"
#include "util/noncopyable.h"

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

class AsioNetServer : public noncopyable
{
public:
    explicit AsioNetServer(ILogicServer* server);

    virtual ~AsioNetServer() = default;

    virtual NetServerType GetType() const = 0;
    // 初始化
    virtual bool Init(const IpAddress& addr, int32_t thread_num) = 0;
    // 开启服务, 将进入循环
    virtual void Loop() = 0;
    // 退出
    virtual void Exit() = 0;

    // 监听信号
    void SetSignal(int sig);
    // 移除信号
    void DelSignal(int sig);
    // 信号触发
    void OnSignal(int sig);

    TimerManager& GetTimerManager() {return timer_manager_; }

protected:
    ILogicServer* logic_server_{nullptr};
    TimerManager timer_manager_;
    sigset_t concern_signal_mask_{};


private:

};





}
}
#endif //WUKONG_ASIO_SERVER_H
