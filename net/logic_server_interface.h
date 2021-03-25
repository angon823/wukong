#ifndef WUKONG_LOGIC_SERVER_INTERFACE_H
#define WUKONG_LOGIC_SERVER_INTERFACE_H
#include "connection.h"
#include "define.h"
#include "log/log.h"
#include "asio_server_factory.h"

namespace wukong
{

class ILogicServer
{
public:
    virtual void ConnectionCallBack(const net::ConnectionSPtr &) = 0;
    virtual void OnMessageRecv(net::ConnectionSPtr &) = 0;
    virtual void OnSignal(int signal) = 0;

    // FIXME it's needed?
    // virtual void Tick(int32_t delta_time) {}
};

class DefaultServer : public ILogicServer
{
public:
    void Create(const net::IpAddress& addr)
    {
        server_ = net::AsioServerFactory::CreateAsioNetServer(this, addr);
        server_->SetSignal(SIGINT);
        server_->SetSignal(SIGHUP);
    }

    void MainLoop()
    {
        if (server_)
        {
            server_->Loop();
            delete server_;
        }
    }

    void ConnectionCallBack(const net::ConnectionSPtr& con) override
    {
        LogInfo("con:%s, state:%d", con->ToString().c_str(), con->GetState());
    }

    void OnMessageRecv(net::ConnectionSPtr& con) override
    {
        auto* buffer = con->RecvBuffer();
        if (buffer->Size() <= 4)
        {
            LogInfo("size less 4");
            return;
        }

        auto len = buffer->PeakUint32();
        if (len > 11111)
        {

        }

        if (buffer->Size() < len + 4)
        {
            return;
        }

        if (buffer->Size() > len +4)
        {
            LogInfo("========================================== size:%d, len:%d", buffer->Size(), len+4);
        }

        auto slice = buffer->Get(len+4);
        if (slice == nullptr)
        {
            LogError("con:%s, msg null!", con->ToString().c_str());
            return;
        }

        slice->Data_ += 4;

//        LogInfo("con:%s, msg:%s", con->ToString().c_str(), std::string(slice->Data_, len).c_str());
        if (slice->Data_[0] == 'p')
        {
            con->Send("pong pong pong pong pong pong pong pong pong pong pong pong pong ");
        }
        else
        {
            con->Send("thanks, I received your message");
        }
    }

    void OnSignal(int signal) override
    {
        server_->Exit();
    }

private:
    net::AsioNetServer* server_{};
};

} // namespace wukong



#endif //WUKONG_LOGIC_SERVER_INTERFACE_H
