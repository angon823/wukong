#include "buffer.h"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace wukong::net;
uint32_t RoundUpPowerOf2(uint32_t val)
{
    if ((val & (val - 1)) == 0)
    {
        return val;
    }

    uint32_t ret = 1;
    while (ret < val) ret <<= 1;
    return ret;
}

Buffer::Buffer(uint32_t size)
    :size_(RoundUpPowerOf2(size))
{
    assert(size > 0);
    buffer_ = new char[size_];
    in_ = out_ = 0;
}

Buffer::~Buffer()
{
    if (nullptr != buffer_)
    {
        delete[] buffer_;
        buffer_ = nullptr;
    }
    in_ = out_ = 0;
}

bool Buffer::Put(const char *data, uint32_t len)
{
    auto in = in_.load();
    auto out = out_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    if (size_ - (in - out) < len)
    {
        return false;
    }

    uint32_t l = std::min(len, size_ - (in  & (size_ - 1)));
    memcpy(buffer_ + (in & (size_ - 1)), data, l);
    if (l < len)
    {
        memcpy(buffer_, data + l, len - l);
    }

    std::atomic_thread_fence(std::memory_order_release);
    in_ = in + len;
    return true;
}

uint32_t Buffer::Get(char *out_buffer, uint32_t len)
{
    auto in = in_.load();
    auto out = out_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    if (len > in - out)
        return 0;

    uint32_t l = std::min(len, size_ - (out & (size_ - 1)));
    memcpy(out_buffer, buffer_ + (out & (size_ - 1)), l);
    if (l < len)
    {
        memcpy(out_buffer + l, buffer_, len - l);
    }

    std::atomic_thread_fence(std::memory_order_release);
    out_ = out + len;
    return len;
}


uint32_t Buffer::GetFull(char *out_buffer)
{
    auto in = in_.load();
    auto out = out_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    if (in == out)
        return 0;

    auto len = in - out;
    if (len > in - out)
        len = in - out;

    uint32_t l = std::min(len, size_ - (out & (size_ - 1)));
    memcpy(out_buffer, buffer_ + (out & (size_ - 1)), l);
    if (l < len)
    {
        memcpy(out_buffer + l, buffer_, len - l);
    }

    std::atomic_thread_fence(std::memory_order_release);
    out_ = out + len;
    return len;
}

Buffer::CharSliceSPtr Buffer::GetAll()
{
    auto in = in_.load();
    auto out = out_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    auto len = in - out;
    if (len == 0)
        return nullptr;

    auto slice =  std::make_shared<CharSlice>(len);

    uint32_t l = std::min(len, size_ - (out & (size_ - 1)));
    slice->append(buffer_ + (out & (size_ - 1)), l);
    if (l < len)
    {
        slice->append(buffer_, len -l);
    }

    std::atomic_thread_fence(std::memory_order_release);
    out_ = out + len;
    return slice;
}

Buffer::CharSliceSPtr Buffer::Get(uint32_t len)
{
    auto in = in_.load();
    auto out = out_.load();
    std::atomic_thread_fence(std::memory_order_acquire);
    if (len > in - out)
        return nullptr;

    auto slice =  std::make_shared<CharSlice>(len);

    uint32_t l = std::min(len, size_ - (out & (size_ - 1)));
    slice->append(buffer_ + (out & (size_ - 1)), l);
    if (l < len)
    {
        slice->append(buffer_, len -l);
    }

    std::atomic_thread_fence(std::memory_order_release);
    out_ = out + len;
    return slice;

}

uint32_t Buffer::GetWithFunc(const std::function<uint32_t(char*, uint32_t)>& func)
{
    auto in = in_.load();
    auto out = out_.load();
    std::atomic_thread_fence(std::memory_order_acquire);

    if (in == out)
        return 0;

    auto len = in - out;
    uint32_t l = std::min(len, size_ - (out & (size_ - 1)));
    func(buffer_ + (out_ & (size_ - 1)), l);
    if (l < len)
    {
        func(buffer_, len - l);
    }

    std::atomic_thread_fence(std::memory_order_release);
    out_ = out + len;
    return 0;
}

uint32_t Buffer::GetUInt32()
{
    char out_buffer[4];
    return ntohl(uint32_t(Get(out_buffer, 4)));
}

int32_t Buffer::ReadFd(int32_t fd, ErrCode& errCode)
{
    int32_t recv_bytes = 0;
    do
    {
        auto in = in_.load();
        auto out = out_.load();
        std::atomic_thread_fence(std::memory_order_acquire);

        if (size_ - (in - out) == 0)
        {
            errCode = ErrBufferNotEnough;
            break;
        }

        int32_t n = ::recv(fd, buffer_ + (in & (size_ - 1)), size_ - (in - out), 0);
        if (n == 0) break;
        if (n < 0)
        {
            if (errno != EAGAIN)
            {
                perror("ReadFd error");
                errCode = ErrSocketErr;
                return -1;
            }
            break;
        }
        recv_bytes += n;
        std::atomic_thread_fence(std::memory_order_release);
        in_ = in + n;
    } while (true);
    return recv_bytes;
}

// call in main thread. write
int32_t Buffer::SendFd(int32_t fd, const char *data, size_t len)
{
    int32_t send_bytes = 0;
    while (send_bytes < int32_t(len))
    {
        int32_t n = ::send(fd, data, len, 0);
        if (n < int32_t(len))
        {
            if (n == -1 && errno != EAGAIN)
            {
                perror("write error");
                return  -1;
            }
            break;
        }
        send_bytes += n;
    }

    if (send_bytes < int32_t(len))
    {
        if (!Put(data + send_bytes, (send_bytes -len)))
        {
            return -1;
        }
    }

    return send_bytes;
}

// call in sub thread loop. read
int32_t Buffer::SendSelf(int32_t fd)
{
    int32_t send_bytes = 0;
    while (true)
    {
        auto in = in_.load();
        auto out = out_.load();
        std::atomic_thread_fence(std::memory_order_acquire);

        auto len = in - out;
        if (len == 0)
            break;

        auto l = size_ - (out & (size_ - 1));
        auto n = ::send(fd, buffer_ + (out & (size_ - 1)), l, 0);
        if (n < l)
        {
            if (n == -1 && errno != EAGAIN)
            {
                perror("write error");
                return  -1;
            }
            if (n > 0)
            {
                send_bytes += n;
                std::atomic_thread_fence(std::memory_order_release);
                out_ = out + n;
            }
            break;
        }

        if (l < len)
        {
            n = ::send(fd, buffer_, len - l, 0);
            if (n < len - l)
            {
                if (n == -1 && errno != EAGAIN)
                {
                    perror("write error");
                    return  -1;
                }
                if (n > 0)
                {
                    send_bytes += n;
                    std::atomic_thread_fence(std::memory_order_release);
                    out_ = out+ n;
                }
                break;
            }
        }

        send_bytes += n;
        std::atomic_thread_fence(std::memory_order_release);
        out_ = out + len;
    }

    return send_bytes;
}

uint32_t Buffer::Size() const
{
    std::atomic_thread_fence(std::memory_order_acquire);
    return in_.load() - out_.load();
}



