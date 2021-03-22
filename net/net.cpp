#include <iostream>
#include "tcp_server.h"

void signal_handler(int sig)
{
    std::cout << "sig\n";
}

void ctrl_c_op( int signo )
{
    std::cout << "caught SIGUSR1, no : " << signo   << std::endl;
}

int main()
{
    std::cout << "ready start！\n";

//    signal(SIGINT, signal_handler);

    struct sigaction act{};
    act.sa_handler=ctrl_c_op;
    sigemptyset(&act.sa_mask);

    act.sa_flags=0;

    auto i = sigaction( SIGINT,&act, nullptr ) ;
    if ( i != 0 )
    {
        std::cout << "sigaction failed ! iRet : " << i  << std::endl;
        return -1;
    }

    wukong::net::IpAddress addr(1234);
    wukong::net::CreateAndServeTcpServer(addr);


    return 0;
}