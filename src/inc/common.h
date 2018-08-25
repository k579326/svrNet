

#pragma once

#include "engine.h"


typedef struct send_helper_t send_helper_t;

typedef struct recv_helper_t recv_helper_t;

typedef struct conn_table_t conn_table_t;

typedef struct net_svr_t
{
	// 服务器监听socket
	int             svr_sock;
	
	// 处理IO的epoll的fd
	int             ep_fd;
	
	// 监听客户端连接的线程ID
	pthread_t       lis_tid;
	
	// epoll处理IO事件的线程ID
	pthread_t       loop_tid;
	
	// 连接表，存储当前连接的客户端
	conn_table_t*	conntable;

	// 线程池，负责处理客户端请求
    work_pool_t 	pool;
	
	// 数据发送辅助
	send_helper_t* 	shpr;
	
	// 数据接收辅助
	recv_helper_t*	rhpr;
}net_svr_t;









