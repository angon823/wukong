#ifndef WUKONG_LOGIC_SERVER_INTERFACE_H
#define WUKONG_LOGIC_SERVER_INTERFACE_H
#include "connection.h"
#include "define.h"
#include "../base/log.h"

namespace wukong
{

class ILogicServer
{
public:
    virtual void ConnectionCallBack(const net::ConnectionSPtr &) = 0;
    virtual void OnMessageRecv(net::ConnectionSPtr &) = 0;
};

class defaultServer : public ILogicServer
{
public:
    void ConnectionCallBack(const net::ConnectionSPtr & con) override
    {
        LogInfo("con:%s, isConnect:%d", con->ToString().c_str(), con->GetState());
    }

    void OnMessageRecv(net::ConnectionSPtr & con) override
    {
        auto slice = con->RecvBuffer().GetAll();
        LogInfo("con:%s, msg:%s", con->ToString().c_str(), slice->Data_);
        con->Send("thanks, I received your message");
    }
};

} // namespace wukong



#endif //WUKONG_LOGIC_SERVER_INTERFACE_H
