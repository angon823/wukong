#ifndef WUKONG_LOGIC_SERVER_INTERFACE_H
#define WUKONG_LOGIC_SERVER_INTERFACE_H
#include "connection.h"
#include "define.h"
#include "log/log.h"

namespace wukong
{

class ILogicServer
{
public:
    virtual void ConnectionCallBack(const net::ConnectionSPtr &) = 0;
    virtual void OnMessageRecv(net::ConnectionSPtr &) = 0;

    // FIXME it's needed?
    // virtual void Tick(int32_t delta_time) {}
};

class defaultServer : public ILogicServer
{
public:
    void ConnectionCallBack(const net::ConnectionSPtr & con) override
    {
        LogInfo("con:%s, state:%d", con->ToString().c_str(), con->GetState());
    }

    void OnMessageRecv(net::ConnectionSPtr & con) override
    {
        auto slice = con->RecvBuffer().GetAll();
        if (slice == nullptr)
        {
            LogError("con:%s, msg null!", con->ToString().c_str());
            return;
        }
        LogInfo("con:%s, msg:%s", con->ToString().c_str(), slice->Data_);
        con->Send("thanks, I received your message");
    }
};

} // namespace wukong



#endif //WUKONG_LOGIC_SERVER_INTERFACE_H
