#include "asio_server.h"
#include "logic_server_interface.h"
#include "util/utility.h"

using namespace wukong::net;



void signal_handler(int signo);

class CaughtSignal
{
public:

    // FIXME，从epoll读取信号
    explicit CaughtSignal()
    {
        wukong::Signal(SIGINT, signal_handler);
        wukong::Signal(SIGHUP, signal_handler);
        wukong::Signal(SIGPIPE, signal_handler);
        wukong::Signal(SIGUSR1, signal_handler);
        wukong::Signal(SIGUSR2, signal_handler);
    }

    void OnSignal(int sig) const
    {
        if (handler_)
        {
            handler_->OnSignal(sig);
        }
    }

    AsioNetServer* handler_{nullptr};
};

static CaughtSignal init;

inline void signal_handler(int signo)
{
    init.OnSignal(signo);
}


AsioNetServer::AsioNetServer(wukong::ILogicServer *server)
        : logic_server_(server)
{
    timer_manager_.Init();
    sigemptyset(&concern_signal_mask_);
    init.handler_ = this;
}

void AsioNetServer::SetSignal(int sig)
{
    sigaddset(&concern_signal_mask_, sig);
}


void AsioNetServer::DelSignal(int sig)
{
    sigdelset(&concern_signal_mask_, sig);
}

void AsioNetServer::OnSignal(int sig)
{
    // todo 当前调用是非线程安全的
    if (sigismember(&concern_signal_mask_, sig))
    {
        logic_server_->OnSignal(sig);
    }
}

