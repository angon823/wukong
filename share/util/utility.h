#ifndef WUKONG_UTILITY_H
#define WUKONG_UTILITY_H
#include <csignal>

namespace wukong
{
    typedef void (*sighandler_t)(int);
    typedef void (*sighandler_t_new)(int, siginfo_t*, void *);
    inline sighandler_t Signal(int signum, sighandler_t handler, sighandler_t_new new_handler = nullptr)
    {
        struct sigaction action{}, old_action{};

        if (handler != nullptr)
        {
            action.sa_handler = handler;
            action.sa_flags = 0;
        }
        else
        {
            action.sa_flags |= SA_SIGINFO;
            action.sa_sigaction = new_handler;
        }

        sigemptyset(&action.sa_mask);
        if (sigaction(signum, &action, &old_action) < 0)
        {
            perror("sigaction");
            exit(-1);
        }
        return (old_action.sa_handler);
    }

}
#endif //WUKONG_UTILITY_H
