#include <iostream>

#include "tcp_server.h"

#include <thread>

int test(){
    int b = 0;
    int a= 0;
    std::thread x([&](){
        b = 42;
        a = 1;
    });
    std::thread y([&](){
        printf("%d ", a);
        printf("%d\n",  b);

    });
    x.join();
    y.join();

    return 0;
}

int main()
{
    for (int i = 0; i < 100000; ++i)
    {
        test();
//        printf("-----------------------\n");
    }

//    std::cout << "ready start！\n";
//    wukong::net::IpAddress addr(1234);
//    wukong::net::CreateAndServeTcpServer(addr);

    return 0;
}