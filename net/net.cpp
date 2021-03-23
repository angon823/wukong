#include <iostream>
#include "tcp_server.h"
#include "asio_server_factory.h"
#include "logic_server_interface.h"

using namespace wukong::net;

AsioNetServer* tcpserver = nullptr;

void signal_handler(int sig)
{
    std::cout << "sig\n";
}

void ctrl_c_op( int signo )
{
    if (tcpserver)
    {
        tcpserver->Uninit();
    }
    std::cout << "caught SIGUSR1, no : " << signo   << std::endl;
}

int main()
{
    std::cout << "ready start！\n";

    struct sigaction act{};
    act.sa_handler=ctrl_c_op;
    sigemptyset(&act.sa_mask);

    act.sa_flags=0;


    auto i = sigaction(SIGINT, &act, nullptr ) ;
    if ( i != 0 )
    {
        std::cout << "sigaction failed ! iRet : " << i  << std::endl;
        return -1;
    }

    IpAddress addr(1234);
    tcpserver = AsioServerFactory::CreateAsioNetServer(new wukong::DefaultServer(), addr);
    if (tcpserver)
    {
        tcpserver->Loop();
    }

    return 0;
}