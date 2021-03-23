cmake_minimum_required(VERSION 3.1)
project(net)

#set(CMAKE_CXX_STANDARD 14)

include_directories(${WUKONG_SOURCE_DIR}/share)
include_directories(${WUKONG_SOURCE_DIR}/third)
include_directories(${WUKONG_SOURCE_DIR}/net)


set(net_HEAD
        poller/epoll.h
        poller/poll_event.h
        listener.h
        socket.h
        thread_pool.h
        thread_loop.h
        buffer.h
)

file(GLOB_RECURSE net_SRC *.cpp)

message(STATUS "net : ${WUKONG_SOURCE_DIR}/share/timer/")

list(APPEND net_SRC
        ${WUKONG_SOURCE_DIR}/share/timer/time_manager.cpp
        ${WUKONG_SOURCE_DIR}/share/timer/time_manager.h
    )


add_executable(net ${net_SRC} asio_server_factory.cpp asio_server_factory.h)
