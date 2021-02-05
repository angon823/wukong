#ifndef __H_Acceptor_H__
#define __H_Acceptor_H__


#include "ip_address.h"
#include "socket.h"

namespace wukong {
namespace net {

class Acceptor {
public:
	void SetSocket(Socket sock);
	int32_t Bind(const IpAddress & addr);
	int32_t Listen();

	Socket Accept(IpAddress *addr);


private:
	Socket sock_;
};


}
}




#endif
