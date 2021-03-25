#include <iostream>
#include "tcp_server.h"
#include "asio_server_factory.h"
#include "logic_server_interface.h"
#include "util/utility.h"

using namespace wukong::net;

//AsioNetServer* tcpserver = nullptr;

auto server =  new wukong::DefaultServer();

void ctrl_c_op( int signo )
{
    std::cout << "caught signo : " << signo   << std::endl;
    server->OnSignal(signo);
}

int main()
{
    std::cout << "ready start！\n";

//    wukong::Signal(SIGINT, ctrl_c_op);

    server = new wukong::DefaultServer();
    server->Create(IpAddress(1234));
    server->MainLoop();

    return 0;
}