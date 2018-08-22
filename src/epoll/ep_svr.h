#pragma once

#include <arpa/inet.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include "engine.h"



typedef struct
{
	unsigned char		iptype;			// ipv4: 0, ipv6 1
	unsigned char		ip[16];
	int                 client_fd;
}clt_info_t;


typedef struct 
{
	clt_info_t	clt;
	int 		conntype;		// 连接类型，区分不同的网络任务
}svr_connect_t;


typedef void* conn_table_t;

struct es_svrinfo_t
{
	int             sock_listen;
	int             ep_fd;
	pthread_t       listen_tid;
	pthread_t       work_tid;
	connect_table	conntable;
    struct work_pool_t pool;
	sender_thread sender;
};




int init_env(struct es_svrinfo_t* info);

int uninit_env(struct es_svrinfo_t* info);

void* listen_thread(void* param);

void* work_routine(void* param);



