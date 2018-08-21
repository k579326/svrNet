
#include <sys/sysinfo.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include "Glist.h"
#include "engine.h"
#include "ep_svr.h"	// 优化
#include "net_config.h"
#include "pack.h"

typedef struct
{
	struct work_pool_t* pool;
	int 		 socket;				// 从此socket缓冲区接收到的数据
	unsigned int size;
	void* 		 data;
}work_param_t;




static void* sendthread_func(void* param)
{
	sender_thread* st = (sender_thread*)param;

	while (1)
	{
		struct epoll_event ep_evt;
		int socket = 0;
		
		// 查找缓冲区中的第一个数据
		socket = QSend_Socket_of_front();
		if (socket == -1)
		{
			pthread_mutex_lock(&st->lock);
			pthread_cond_wait(&st->cond, &st->lock);
			pthread_mutex_unlock(&st->lock);

			continue;
		}

		// TODO:线程退出信号
		if (0)
		{
			break;
		}
		
		ep_evt.events = EPOLLIN | EPOLLET | EPOLLOUT;
		ep_evt.data.fd = socket;
		if (epoll_ctl(st->epfd, EPOLL_CTL_MOD, socket, &ep_evt) != 0)
		{
			printf("[EPOLLMODIFY] modify socket %d epoll event failed!\n");
			assert(0);
		}
	}

	return;
}


static void poolthread_func(void* param)
{
	work_param_t* work = (work_param_t*)param;
	net_pkg_t* outdata = NULL;
	net_pkg_t* pack = (net_pkg_t*)work->data;
	int outlen = 0;
	int msgID = 0;
	int ret;

	assert(pack->length + sizeof(net_pkg_t) == work->size);
	
	if (work->pool->work_func)
	{
		outdata = (net_pkg_t*)malloc(NET_PACK_MAX_SIZE);
		outlen = NET_PACK_MAX_SIZE;
		ret = work->pool->work_func(pack->data, pack->length, (void*)outdata, &outlen);
		if (ret == 0)
		{
			msgID = pack->msgID;
			outlen = create_pack(outdata, outlen, msgID);
			QSend_push(work->socket, outdata, outlen);
			// 通知发送线程，有活干了
			pthread_cond_signal(&work->pool->send_thread->cond);
		}
		free(outdata);
		outdata = NULL;
	}

	return;
}


static void thread_completed(void* param)
{
	work_param_t* work = (work_param_t*)param;

	if (work->pool->after_wkfunc)
	{
		//TODO: work->pool->after_wkfunc(,);
	}

	release_pack(work->data);
	free(param);
	
	return;
}

static void thread_cancel(void* param)
{
	work_param_t* work = (work_param_t*)param;

	if (work->pool->cancel_func)
	{
		// TODO:work->pool->cancel_func(,);
	}

	release_pack(work->data);
	free(param);
	
	return;
}


int workpool_init(struct work_pool_t* wp, int poolsize,	task_cb w, after_work_cb aw, cancel_works_cb cw)
{
	if (poolsize <= 0)
		poolsize = get_nprocs() * 2 + 1;
	
	wp->poolsize = poolsize;

	threadpool_init(&wp->thread_pool, wp->poolsize);

	wp->work_func = w;
	wp->after_wkfunc = aw;
	wp->cancel_func = cw;

	return 0;
}

int workpool_dowork(struct work_pool_t* wp, int cltsock, void* data, int len)
{
	work_param_t* buf = (work_param_t*)malloc(sizeof(work_param_t));

	buf->pool = wp;
	buf->size = len;
	buf->data = data;
	buf->socket = cltsock;
	
	return threadpool_push_work(wp->thread_pool, buf, poolthread_func, NULL);
}


int workpool_uninit(struct work_pool_t* wp)
{
	threadpool_uninit(wp->thread_pool, wp->cancel_func);
	return 0;
}




int sendthread_init(sender_thread* st, int epoll_fd)
{
	pthread_mutex_init(&st->lock, NULL);
	pthread_cond_init(&st->cond, NULL);
	st->epfd = epoll_fd;
	
	return 0;
}
int sendthread_start(sender_thread* st)
{
	pthread_create(&st->id, NULL, sendthread_func, st);

	return 0;
}

int sendthread_uninit(sender_thread* st)
{
	pthread_mutex_destroy(&st->lock);
	pthread_cond_destroy(&st->cond);

	return 0;
}









