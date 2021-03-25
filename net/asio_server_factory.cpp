#include "asio_server_factory.h"
using namespace wukong::net;

AsioNetServer *AsioServerFactory::CreateAsioNetServer(ILogicServer *i_logic_server, const IpAddress& address, NetServerType type)
{
    AsioNetServer* server = nullptr;
    switch (type)
    {
        case NetServerType::TCP:
        {
            server = new TcpServer(i_logic_server);
            if (!server->Init(address, 1))
            {
                server->Exit();
                delete server;
                server = nullptr;
                break;
            }
        }
        case NetServerType::UDP:
        {
            // todo
        }
        case NetServerType::KCP:
        {
            // todo
        }
    }
    return server;
}
