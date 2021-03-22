#ifndef WUKONG_MQ_H
#define WUKONG_MQ_H
#include <vector>
#include <memory>
#include <cstring>
#include "lockfree/readerwriterqueue.h"

namespace wukong
{

template<typename T>
class ReaderWriterMQ
{
public:

    void PushBack(const T &t)
    {
        msgs_.enqueue(t);
    }

    T PopFront()
    {
        T t;
       msgs_.try_dequeue(t);
       return t;
    }

private:
    moodycamel::ReaderWriterQueue<T, 1024> msgs_;
};

}

#endif //WUKONG_MQ_H
