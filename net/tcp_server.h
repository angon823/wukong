#ifndef __H_TCP_SERVER_H__
#define __H_TCP_SERVER_H__

#include <unordered_map>
#include <map>
#include <memory>
#include <atomic>
#include "socket.h"
#include "define.h"
#include "listener.h"
#include "poller/poller.h"
#include "asio_server.h"
#include "message_procceor.h"

namespace wukong {
class ILogicServer;

namespace net {

class ThreadPool;
class TcpServer : public AsioNetServer
{
public:
	explicit TcpServer(ILogicServer * logic_server);
    ~TcpServer() override;
    NetServerType GetType() const override {return NetServerType::TCP;}

	bool Init(const IpAddress& addr, int32_t thread_num) override;
    void Loop() override;
	void Exit() override;
    bool CloseConnection(const ConnectionSPtr& con);

private:
	ConnectionSPtr GetConnection(int32_t fd);

    void handleListenEvent(int32_t fd, uint32_t events);
    void handleListenErr(int32_t fd);
    void handleListenRead(int32_t fd);

    void onNewConnectionAccept(const Socket& sock, const IpAddress& addr);
    void onNewConnectionInitFinish(int32_t fd, int32_t err);
    void onConnectionDisconnect(int32_t fd);
    void onConnectionBad(int32_t fd, int32_t err_code);

	bool addConnection(const ConnectionSPtr& con);
	bool removeConnection(int32_t fd);

	void registerMessage();

    void poll();
    void processMsg();
    void handleTimer();
private:
    std::atomic<bool> quit_{false};
	std::unique_ptr<Listener> listener_{nullptr};
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unordered_map<int32_t, ConnectionSPtr> connections_;
    MessageProcessor msg_processor_;
};

}
}


#endif