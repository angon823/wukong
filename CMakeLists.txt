cmake_minimum_required(VERSION 3.1)
project(wukong)

#set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -pthread -Wall")

message(STATUS "wukong")

set(WUKONG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${WUKONG_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${WUKONG_SOURCE_DIR}/lib)

message(STATUS "Server Bin path: ${SERVER_BINARY_DIR}")
message(STATUS "Exe path: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "Src path: ${WUKONG_SOURCE_DIR}")


include_directories(${WUKONG_SOURCE_DIR}/util)
include_directories(${WUKONG_SOURCE_DIR}/third)
include_directories(${WUKONG_SOURCE_DIR}/third/lockfree)



#add_subdirectory(base)
add_subdirectory(net)
