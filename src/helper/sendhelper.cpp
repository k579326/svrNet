
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <list>
#include <string>
#include <algorithm>

#include <pthread.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "sendhelper.h"
#include "config.h"
#include "common.h"
#include "conn.h"

using namespace std;


typedef string DATA;

// 发送缓冲区
typedef struct
{
	CONNID 	    conn_id;
	DATA 		data;
}_SEND_T;


typedef struct send_queue_t
{
	pthread_mutex_t 		m_lock;
	list<_SEND_T> 			m_list;

	send_queue_t() 
    { 
        pthread_mutex_init(&m_lock, NULL); 
    }
	~send_queue_t()
    { 
        pthread_mutex_destroy(&m_lock); 
    }

	void lock() 
    { 
        pthread_mutex_lock(&m_lock); 
    }
	void unlock() 
    { 
        pthread_mutex_unlock(&m_lock); 
    }
	
}send_queue_t;



//// 发送数据辅助线程
static void* send_func(void* param)
{
	send_thread_t* thread = (send_thread_t*)param;

	while (1)
	{
		struct epoll_event ep_evt;
		CONNID connid = 0;
		int socket;
		// 查找缓冲区中的第一个数据
		connid = QSend_connid_of_front(thread->cache);
		if (connid == SVR_INVALID_CONNECT_ID)
		{
			send_wait(thread);
			continue;
		}

        socket = conn_get_sockfd(thread->svr->conntable, connid);
        if (socket == -1)
        {// 连接已经不存在
            continue;
        }
        
		// TODO:线程退出信号
		if (0)
		{
			break;
		}
		
		ep_evt.events = EPOLLIN | EPOLLET | EPOLLOUT;
		ep_evt.data.u64 = connid;
		if (epoll_ctl(thread->svr->ep_fd, EPOLL_CTL_MOD, socket, &ep_evt) != 0)
		{
			printf("[EPOLLMODIFY] modify connid %d epoll event failed!\n");
			QSend_remove_all_by_connid(thread->cache, connid);	// 可能已经被清理了，多检查一次
			conn_remove(thread->svr->conntable, connid);		// 可能已经被清理了，多检查一次
			// 接收缓冲区清理在epollIN事件中处理
		}

		usleep(1000 * 10);
	}

	return NULL;
}

static int sendhelper_start(send_thread_t* thread)
{
	pthread_create(&thread->id, NULL, send_func, thread);
	return 0;
}


send_helper_t* create_send_helper(const net_svr_t* svr)
{
	send_helper_t *helper = new send_helper_t();
	helper->thread = new send_thread_t();
    
	pthread_mutex_init(&helper->thread->lock, NULL);
	pthread_cond_init(&helper->thread->cond, NULL);
	helper->thread->svr = (net_svr_t*)svr;
	helper->m_queue = new send_queue_t();
    helper->thread->cache = helper->m_queue;
	helper->start = sendhelper_start;
    
	return helper;
}


int QSend_push(send_queue_t* queue, CONNID connectId, void* data, int len)
{
	_SEND_T x;
	bool hit = false;
	
	queue->lock();

	x.conn_id = connectId;
	x.data.assign((char*)data, len);

	auto it = queue->m_list.begin();
	if (it != queue->m_list.end() && it->conn_id == connectId)
	{
		hit = true;
	}
	
	if (hit && (it->data.size() + len < NET_SEND_BUFF_SIZE))			// 多个小包可以打包发送
	{
		it->data += x.data;	
	}
	else
	{
		queue->m_list.push_back(x);
	}
	
	queue->unlock();

	return 0;
}

int QSend_pop_first(send_queue_t* queue, CONNID conn_id, void* data, int* len)
{
	int ret = 0;

	queue->lock();

	auto it = find_if(queue->m_list.begin(), queue->m_list.end(), [&](const _SEND_T& element)->bool{ return conn_id == element.conn_id;});
	if (it == queue->m_list.end())
	{
		queue->unlock();
		return -1;
	}
	
	if (*len < it->data.size())
	{
		ret = -1;
		*len = it->data.size();
	}
	else
	{
		*len = it->data.size();
		memcpy(data, it->data.c_str(), it->data.size());
		queue->m_list.erase(it);
		ret = 0;
	}
	
	queue->unlock();
	
	return ret;
}

CONNID QSend_connid_of_front(send_queue_t* queue)
{
	CONNID conn_id = SVR_INVALID_CONNECT_ID;
	queue->lock();

	auto it = queue->m_list.begin();
	if (it != queue->m_list.end())
	{
		conn_id = it->conn_id;
	}
	queue->unlock();

	return conn_id;
}

int QSend_remove_all_by_connid(send_queue_t* queue, CONNID connid)
{
	queue->lock();

	for (auto it = queue->m_list.begin(); it != queue->m_list.end(); )
	{
		if (it->conn_id == connid)
		{
			it = queue->m_list.erase(it);
		}
		else
		{
			it++;
		}
	}	
	
	queue->unlock();

	return 0;
}


int QSend_size(send_queue_t* queue)
{
	int size = 0;
	queue->lock();
	size = queue->m_list.size();
	queue->unlock();

	return size;
}


void send_wait(send_thread_t* thread)
{
    pthread_mutex_lock(&thread->lock);
    pthread_cond_wait(&thread->cond, &thread->lock);
    pthread_mutex_unlock(&thread->lock);

    return;
}



void send_signal(send_thread_t* thread)
{
    pthread_cond_signal(&thread->cond);
    return;
}




int release_send_helper(send_helper_t* helper)
{
    delete helper->m_queue;
    helper->m_queue = NULL;
    helper->thread->cache = NULL;
    
	pthread_mutex_destroy(&helper->thread->lock);
	pthread_cond_destroy(&helper->thread->cond);

	delete helper->thread;
    helper->thread = NULL;
	
    delete helper;
    
	return 0;
}













