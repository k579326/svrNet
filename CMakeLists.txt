
cmake_minimum_required(VERSION 3.0)

set(CUSTOM_OS_LINUX ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories(include)
include_directories(third/libuv/include)
include_directories(third/queue)
include_directories(third/threadpool)

add_subdirectory(third/queue)
add_subdirectory(third/libuv)
add_subdirectory(third/threadpool)

add_subdirectory(src)
add_subdirectory(test)