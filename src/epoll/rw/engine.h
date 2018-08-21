
#pragma once

#include "threadpool.h"


typedef int (*task_cb)(const void* in_data, int in_len, void* out_data, int* out_len);


typedef struct
{
	int				epfd;
	pthread_t 		id;
	pthread_mutex_t lock;
	pthread_cond_t 	cond;
}sender_thread;


struct work_pool_t
{
    threadpool_t*   thread_pool;
	sender_thread*  send_thread;
    int             poolsize;  // 提供给使用者的线程数（不包含监听线程和epoll线程）。
    task_cb         work_func;
    after_work_cb   after_wkfunc;
    cancel_works_cb cancel_func;
};


int workpool_init(struct work_pool_t* wp, int poolsize,	task_cb w, after_work_cb aw, cancel_works_cb cw);

int workpool_dowork(struct work_pool_t* wp, int cltsock, void* data, int len);

int workpool_uninit(struct work_pool_t* wp);



int sendthread_init(sender_thread* st, int epoll_fd);
int sendthread_start(sender_thread* st);
int sendthread_uninit(sender_thread* st);











