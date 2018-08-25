
#include <sys/sysinfo.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "engine.h"
#include "config.h"
#include "pack.h"
#include "sendhelper.h"
#include "common.h"

typedef struct
{
	work_pool_t* pool;
	CONNID 		 connid;	// 从此连接的缓冲区接收到的数据
	unsigned int size;
	void* 		 data;
}_work_t;



static void work_proc(void* param)
{
	_work_t* work = (_work_t*)param;
	net_pkg_t* outdata = NULL;
	net_pkg_t* pack = (net_pkg_t*)work->data;
	int outlen = 0;
	int msgID = 0;
	int ret;

	assert(pack->length + sizeof(net_pkg_t) == work->size);
	
	if (work->pool->processmsg)
	{
		outdata = (net_pkg_t*)malloc(NET_PACK_MAX_SIZE);
		outlen = NET_PACK_MAX_SIZE;
		ret = work->pool->processmsg(pack->data, pack->length, (void*)outdata, &outlen);
		if (ret == 0)
		{
			msgID = pack->msgID;
			outlen = create_pack(outdata, outlen, msgID);
            
            QSend_push(work->pool->sendthread->cache, work->connid, outdata, outlen);
			// 通知发送线程，有活干了
			send_signal(work->pool->sendthread);
		}
		free(outdata);
		outdata = NULL;
	}

	return;
}


static void thread_completed(void* param)
{
	_work_t* work = (_work_t*)param;

	//TODO: work->pool->after_wkfunc(,);

	release_pack(work->data);
	free(param);
	
	return;
}

static void work_cancel(void* param)
{
	_work_t* work = (_work_t*)param;

	// TODO:work->pool->cancel_func(,);

	release_pack(work->data);
	free(param);
	
	return;
}


int workpool_start(work_pool_t* wp, TASKHANDLER procfunc, int poolsize)
{
	if (poolsize <= 0)
		poolsize = get_nprocs() * 2 + 1;
	
	wp->poolsize = poolsize;
	wp->processmsg = procfunc;
	wp->sendthread = ((net_svr_t*)((char*)wp - (char*)&((net_svr_t*)0)->pool))->shpr->thread;
	
	threadpool_init(&wp->thread_pool, wp->poolsize);
    
	return 0;
}

int workpool_push_work(work_pool_t* wp, CONNID connid, void* data, int len)
{
	_work_t* buf = (_work_t*)malloc(sizeof(_work_t));

	buf->pool = wp;
	buf->size = len;
	buf->data = data;
	buf->connid = connid;
	
	return threadpool_push_work(wp->thread_pool, buf, work_proc, NULL);
}


int workpool_stop(work_pool_t* wp)
{
	threadpool_uninit(wp->thread_pool, work_cancel);
	return 0;
}










