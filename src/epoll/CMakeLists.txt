

include_directories(cache)

add_library(epsvr ep_svr.c ep_svr.h pack.c pack.h
			rw/engine.h rw/engine.c)

target_link_libraries(epsvr pthread Glist netcache threadpool libuv)

add_subdirectory(cache)

