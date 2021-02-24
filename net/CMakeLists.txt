cmake_minimum_required(VERSION 3.1)
project(net)

set(CMAKE_CXX_STANDARD 14)

message(STATUS "Net Watch Src path: ${WUKONG_SOURCE_DIR}/third")


include_directories(${WUKONG_SOURCE_DIR}/third)
include_directories(${WUKONG_SOURCE_DIR}/net)
include_directories(${WUKONG_SOURCE_DIR}/util)


set(net_HEAD
        poller/epoll.h
        poller/poll_event.h
        listener.h
        socket.h
        thread_pool.h
        thread_loop.h
        buffer.h
)

FILE(GLOB_RECURSE net_SRC *.cpp)


add_executable(net ${net_SRC})
