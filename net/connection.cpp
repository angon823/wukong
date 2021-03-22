#include "connection.h"
#include "poller/poll_event.h"
#include <sstream>
#include "log/log.h"
#include "thread_loop.h"
using namespace wukong::net;

Connection::Connection(Socket sock, const IpAddress& addr)
	: sock_(sock)
	, peer_addr_(addr)
{
}

int32_t Connection::Close()
{
	sock_.Close();
    SetState(kConnectionStateClosed);
	return 0;
}

ErrCode Connection::RecvMsg()
{
    ErrCode ret = Success;
    recv_buffer_.ReadFd(sock_.GetFd(), ret);
    return ret;
}

std::string Connection::ToString() const
{
    std::ostringstream ss;
    ss << "fd:" << sock_.GetFd() << " addr:" << peer_addr_.ToString();
    return ss.str();
}

void Connection::SetState(ConnectionState state)
{
    LogDebug("Connection:%s state change[%d->%d]", ToString().c_str(), state_.load(), state);
    state_ = state;
}

int32_t Connection::Send(const char *data, size_t len)
{
    int32_t n = 0;
    // todo
    auto old_len = send_buffer_.Size();
    if (old_len == 0)
    {
        n = send_buffer_.SendFd(GetFd(), data, len);
        if (n < 0)
        {
            return -1;
        }
        if (n < int32_t(len))
        {
            loop_->EmplaceMessage(kDownEnableWrite, GetFd());
            goto high_water_mark;
        }
        return n;
    }

    if (!send_buffer_.Put(data, len))
    {
        return -1;
    }

    high_water_mark:
    auto now_len = send_buffer_.Size();
    if (high_water_mark_callback_ && old_len < high_water_mark_ && now_len > high_water_mark_)
    {
        high_water_mark_callback_(shared_from_this(), now_len);
    }
    return n;
}

int32_t Connection::Send(const std::string &data)
{
    return Send(data.data(), data.length());
}

int32_t Connection::HandleWritable()
{
    if (send_buffer_.Empty())
        return 0;

    int32_t n = send_buffer_.SendSelf(GetFd());
    if (n < 0)
    {
        return n;
    }

    if (send_buffer_.Empty())
    {
        return 0;
    }
    return n;
}
