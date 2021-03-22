#ifndef WUKONG_TIME_H
#define WUKONG_TIME_H


#include <chrono>

namespace wukong
{

int64_t GetNowMs()
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    return ms.count();
};

}


#endif //WUKONG_TIME_H
