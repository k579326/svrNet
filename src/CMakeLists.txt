
include_directories(epoll/rw)
include_directories(epoll)
add_subdirectory(epoll)

add_library(server-netcore svr_start.c)

target_link_libraries(server-netcore epsvr)
