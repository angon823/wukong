#ifndef WUKONG_MESSAGE_H
#define WUKONG_MESSAGE_H

#include <cstdint>
#include <utility>
#include "define.h"

namespace wukong{
namespace net{


enum MessageType
{
    kMessageTypeNone,
    // loop->server
    kUpOnNewConInitFinish,
    kUpOnConMessage,
    kUpOnConDisconnect,
    kUpOnConErr,

    // server->loop
    kDownNewConAccept,
    kDownCloseCon,
    kDownEnableWrite,
};

#define NEW_SP_MSG(evt_ptr) NEW_SHARED_PTR(static_cast<Message*>(evt_ptr))
#define MAKE_SP_MSG(class_name, ...) NEW_SP_MSG(NewObj(class_name, ##__VA_ARGS__))

struct Message
{
public:
    Message() = default;

    virtual  ~Message() = default;

    Message(MessageType typ, int32_t fd)
            : typ_(typ), fd_(fd)
    {}

    MessageType typ_{kMessageTypeNone};
    int32_t fd_{0};
};

class MessageErr : public Message
{
public:
    MessageErr(MessageType typ, int32_t fd, int32_t errcode)
            : Message(typ, fd), errcode_(errcode)
    {}

    int32_t errcode_;
};

struct MessageNewConInitFinish : public Message
{
public:
    MessageNewConInitFinish(int32_t fd, int32_t errcode)
            : Message(kUpOnNewConInitFinish, fd), errcode_(errcode)
    {}

    int32_t errcode_;
};

class Connection;
struct MessageOnNewConAccept : public Message
{
public:
    MessageOnNewConAccept(int32_t fd, ConnectionSPtr con)
            : Message(kDownNewConAccept, fd), con_(std::move(con))
    {}

    ConnectionSPtr con_;
};

};
}

#endif //WUKONG_MESSAGE_H
