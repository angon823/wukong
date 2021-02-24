#ifndef WUKONG_BUFFER_H
#define WUKONG_BUFFER_H
#include <cstdint>
#include <atomic>
#include <cstddef>
#include <functional>
#include <cstring>
#include <memory>
#include "lockfree/atomicops.h"
#include "define.h"

namespace wukong{
namespace net{

// A single-producer, single-consumer lock-free bounded Buffer

constexpr uint32_t defaultRingBufferSize = 1024 * 1024 *2;
constexpr float growUpRate = 1.5;
class Buffer
{
private:
    struct CharSlice
    {
    public:
        explicit CharSlice(uint32_t cap)
                : Len_(cap), Data_(new char[Len_]), index_(0)
        {}

        uint32_t Len_;
        char *Data_{nullptr};

        inline void append(char *data, uint32_t len)
        {
            assert(len + index_ <= Len_);
            memcpy(Data_ + index_, data, len);
        }

    private:
        uint32_t index_;
    };

public:
    using CharSliceSPtr = std::shared_ptr<CharSlice>;

public:
    explicit Buffer(uint32_t size = defaultRingBufferSize);

    ~Buffer();


    uint32_t Get(char *out_buffer, uint32_t len);

    uint32_t GetFull(char *out_buffer);

    CharSliceSPtr GetAll();

    uint32_t GetUInt32();

    CharSliceSPtr Get(uint32_t len);

    uint32_t GetWithFunc(const std::function<uint32_t(char *, uint32_t)> &func);

    bool Put(const char *data, uint32_t len);

    int32_t ReadFd(int32_t fd, ErrCode& errCode);

    int32_t SendFd(int32_t fd, const char *data, size_t len);

    int32_t SendSelf(int32_t fd);

    uint32_t Size() const;

    bool Empty() const { return Size() == 0;}

private:
    char *buffer_{nullptr};              // the buffer holding the data
    const uint32_t size_;                // the size of the allocated buffer
    moodycamel::weak_atomic<uint32_t> in_{0};     // data is added at offset (in % size)
    moodycamel::weak_atomic<uint32_t> out_{0};    // data is extracted from off. (out % size)
};



}
}

#endif //WUKONG_BUFFER_H
