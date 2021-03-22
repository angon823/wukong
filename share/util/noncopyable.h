#ifndef WUKONG_NOCOPYABLE_H
#define WUKONG_NOCOPYABLE_H

namespace wukong
{
    class noncopyable
    {
    public:
        noncopyable(const noncopyable&) = delete;
        void operator=(const noncopyable&) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };
}

#endif //WUKONG_NOCOPYABLE_H
