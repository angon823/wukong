#ifndef WUKONG_ASIO_SERVER_FACTORY_H
#define WUKONG_ASIO_SERVER_FACTORY_H

#include "tcp_server.h"
#include "asio_server.h"

namespace wukong{
namespace net{

class IpAddress;
class AsioServerFactory
{
public:
    static AsioNetServer* CreateAsioNetServer(ILogicServer* i_logic_server, const IpAddress& address, NetServerType type = NetServerType::TCP);
};


}
}
#endif //WUKONG_ASIO_SERVER_FACTORY_H
