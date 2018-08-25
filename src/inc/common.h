

#pragma once

#include "engine.h"


typedef struct send_helper_t send_helper_t;

typedef struct recv_helper_t recv_helper_t;

typedef struct conn_table_t conn_table_t;

typedef struct net_svr_t
{
	// ����������socket
	int             svr_sock;
	
	// ����IO��epoll��fd
	int             ep_fd;
	
	// �����ͻ������ӵ��߳�ID
	pthread_t       lis_tid;
	
	// epoll����IO�¼����߳�ID
	pthread_t       loop_tid;
	
	// ���ӱ��洢��ǰ���ӵĿͻ���
	conn_table_t*	conntable;

	// �̳߳أ�������ͻ�������
    work_pool_t 	pool;
	
	// ���ݷ��͸���
	send_helper_t* 	shpr;
	
	// ���ݽ��ո���
	recv_helper_t*	rhpr;
}net_svr_t;









