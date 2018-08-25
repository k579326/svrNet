

include_directories(inc)
include_directories(conn)
include_directories(core)
include_directories(helper)


add_library(SvrNet STATIC 
			ep_svr.c
			conn/conn.cpp
			conn/conn.h
			core/engine.c
			core/engine.h
			core/pack.c
			core/pack.h
			helper/recvhelper.cpp
			helper/recvhelper.h
			helper/sendhelper.cpp			
			helper/sendhelper.h			
			)
			
target_link_libraries(SvrNet threadpool pthread libuv)


