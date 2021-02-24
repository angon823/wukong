#ifndef WUKONG_I_NET_SERVER_H
#define WUKONG_I_NET_SERVER_H

#include "define.h"

namespace wukong {
class ILogicServer;

namespace net
{

// server enumeration
enum class NetServerType
{
    TCP,
    UDP,
    KCP,
};

class INetServer
{
public:
    explicit INetServer(ILogicServer* server) : server_(server){}

    virtual NetServerType GetType() const = 0;

    virtual void HandleListenRead(int32_t fd) = 0;

    virtual void HandleListenErr(int32_t fd) = 0;

protected:
    ILogicServer* server_{nullptr};
};


//class server
}
}
#endif //WUKONG_I_NET_SERVER_H
