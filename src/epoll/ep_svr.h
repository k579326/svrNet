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



struct clientinfo
{
	easy_list_t*    cltlist; // 客户端列表 TODO: 更换为更快的map
	pthread_mutex_t cltlock;
};


struct es_svrinfo_t
{
	int             sock_listen;
	int             ep_fd;
	pthread_t       listen_tid;
	pthread_t       work_tid;
	struct clientinfo clt;
    struct work_pool_t pool;
	sender_thread sender;
};


typedef struct
{
	struct sockaddr_in  net_info;
	int                 client_fd;
}client_data_t;


int init_env(struct es_svrinfo_t* info);

int uninit_env(struct es_svrinfo_t* info);

void* listen_thread(void* param);

void* work_routine(void* param);



