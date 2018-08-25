
#pragma once

#include "threadpool.h"
#include "conn.h"

typedef int (*TASKHANDLER)(const void* in_data, int in_len, void* out_data, int* out_len);
typedef struct send_thread_t send_thread_t;

typedef struct 
{
    threadpool_t*   thread_pool;
	send_thread_t*  sendthread;
    int             poolsize;       // 提供给使用者的线程数（不包含监听线程和epoll线程）。
	TASKHANDLER		processmsg;		// 任务处理函数
}work_pool_t;


int workpool_start(work_pool_t* wp, TASKHANDLER procfunc, int poolsize);

int workpool_push_work(work_pool_t* wp, CONNID connid, void* data, int len);

int workpool_stop(work_pool_t* wp);










