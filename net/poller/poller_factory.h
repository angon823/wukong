#ifndef WUKONG_POLLER_FACTORY_H
#define WUKONG_POLLER_FACTORY_H
#include "poller.h"
#include "epoll.h"
#include "define.h"

namespace wukong
{
namespace net
{
    static Poller* CreatePoller(NetServerType type)
    {
        Poller *poller = nullptr;
        switch (type)
        {
            case NetServerType::TCP:
            {
                poller = new Epoll();
                break;
            }
            case NetServerType::UDP:
            {
                break;
            }
            case NetServerType::KCP:
                break;
        }
        return poller;
    }
}

}

#endif //WUKONG_POLLER_FACTORY_H
