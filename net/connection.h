#ifndef __H_CONNECTION_H__
#define __H_CONNECTION_H__

#include "socket.h"
#include <memory>
#include "buffer.h"
#include <atomic>
#include "define.h"

namespace wukong {
namespace net {

enum ConnectionState
{
    kConnectionStateNone,
    kConnectionStateConnecting,
    kConnectionStateConnect,
    kConnectionStateDisconnectByServer,
    kConnectionStateDisconnectByPeer,
    kConnectionStateBad,
    kConnectionStateClosed,
};


class PollEvent;
class ThreadLoop;
class Connection : public std::enable_shared_from_this<Connection>
{
public:
	Connection(Socket sock, const IpAddress& addr);

    // Send must be called in main thread
    int32_t Send(const char* data, size_t len);
    int32_t Send(const std::string& data);

	int32_t GetFd() const { return sock_.GetFd(); }
	const IpAddress& GetAddr() const { return peer_addr_; }

    ErrCode RecvMsg();
	int32_t Close();
	int32_t HandleWritable();

	void SetThreadLoop(ThreadLoop* loop) { loop_ = loop;}
    ThreadLoop* GetThreadLoop() const { return loop_; }
    void SetState(ConnectionState state);
    ConnectionState GetState() const { return state_; }
    bool IsConnected() const {return state_ == kConnectionStateConnect;}

	std::string ToString() const;

	Buffer* RecvBuffer() { return &recv_buffer_; }

	void SetHighWaterMarkCallBack(const HighWaterMarkCallBack& cb, uint32_t mark) { high_water_mark_callback_ = cb; high_water_mark_ = mark;}

protected:
    std::atomic<ConnectionState> state_{kConnectionStateNone};
	Socket sock_;
	IpAddress peer_addr_;
    ThreadLoop* loop_{nullptr};
    Buffer recv_buffer_; // sub thread write, main thread read
    Buffer send_buffer_; // sub thread read, main thread write

    uint32_t high_water_mark_{0};
    HighWaterMarkCallBack high_water_mark_callback_{nullptr};
};

}
}

#endif