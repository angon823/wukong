#ifndef __H_TCP_SERVER_H__
#define __H_TCP_SERVER_H__

#include <unordered_map>
#include <map>
#include <memory>
#include <mutex>
#include "socket.h"
#include "define.h"
#include "listener.h"
#include "poller/poller.h"
#include "i_net_server.h"
#include "message_procceor.h"

namespace wukong {
class ILogicServer;

namespace net {

class ThreadPool;
class TcpServer : public INetServer
{
public:
	explicit TcpServer(ILogicServer * logic_server);
    NetServerType GetType() const override {return NetServerType::TCP;}

	int32_t Init(const IpAddress& addr, int32_t thread_num = 1);
	int32_t Serve();
	void Shutdown();
	void SetTimeout(int32_t timeout) { poll_timeout_ms_ = timeout; }
    bool CloseConnection(const ConnectionSPtr& con);
	ConnectionSPtr GetConnection(int32_t fd);

protected:
    void HandleListenErr(int32_t fd) override;
    void HandleListenRead(int32_t fd) override;

private:
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
    void handleSignal();
private:
    bool quit_{false};
	std::unique_ptr<Listener> listener_{nullptr};
    std::unordered_map<int32_t, ConnectionSPtr> connections_;
    std::unique_ptr<ThreadPool> thread_pool_;
    MessageProcessor msg_processor_;
    int32_t poll_timeout_ms_{5};
};


TcpServerSPtr CreateAndServeTcpServer(const IpAddress& addr);

}
}


#endif