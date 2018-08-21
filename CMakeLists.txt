


set(CUSTOM_OS_LINUX ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

cmake_minimum_required(VERSION 2.8)


include_directories(third/threadpool)
include_directories(third/libuv/include)
include_directories(third/queue)
include_directories(third/list)
include_directories(src/epoll/cache)
include_directories(inc)

add_subdirectory(src)
add_subdirectory(third/libuv)
add_subdirectory(third/list)
add_subdirectory(third/queue)
add_subdirectory(third/threadpool)

add_executable(SvrNetTest test.c)

target_link_libraries(SvrNetTest server-netcore)
