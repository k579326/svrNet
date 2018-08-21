
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <list>
#include <map>
#include <string>
#include <algorithm>

#include <pthread.h>

#include "cache.h"



using namespace std;


typedef int CLTSOCKET;
typedef string DATA;


typedef struct _DATA_MAP
{
	pthread_mutex_t 		m_lock;
	map<CLTSOCKET, DATA> 	m_datamap;

	_DATA_MAP() { pthread_mutex_init(&m_lock, NULL); }
	~_DATA_MAP(){ pthread_mutex_destroy(&m_lock); }

	void lock() { pthread_mutex_lock(&m_lock); }

	void unlock() { pthread_mutex_unlock(&m_lock); }
	
}DATA_MAP;




static DATA_MAP s_dataMap;		// TODO:优化，拒绝全局变量



int map_insert(int client_socket, const void* data, int len)
{
	s_dataMap.lock();

	if (s_dataMap.m_datamap.end() != s_dataMap.m_datamap.find(client_socket))
	{
		assert(0);
		s_dataMap.unlock();
		return -1;
	}
		

	s_dataMap.m_datamap[client_socket] = string().assign((char*)data, len);

	s_dataMap.unlock();

	return 0;
}


int map_find(int client_socket, void* data, int* len)
{
	int ret = 0;
	s_dataMap.lock();

	auto it = s_dataMap.m_datamap.find(client_socket);
	if (it == s_dataMap.m_datamap.end())
	{
		ret = -1;
		*len = -1;
	}
	else
	{
		if (it->second.size() > *len)
		{
			ret = -1;
		}
		else
		{
			memcpy(data, it->second.c_str(), it->second.size());
			ret = 0;
		}

		*len = it->second.size();
	}
	
	s_dataMap.unlock();

	return 0;
}


int map_remove(int client_socket)
{
	int ret = 0;
	
	s_dataMap.lock();

	auto it = s_dataMap.m_datamap.find(client_socket);
	if (it == s_dataMap.m_datamap.end())
	{
		// do nothing
	}
	else
	{
		s_dataMap.m_datamap.erase(it);
	}

	s_dataMap.unlock();

	return 0;
}





// 发送缓冲区

typedef struct
{
	CLTSOCKET 	target_socket;
	DATA 		data;
}_SEND_T;


typedef struct _SEND_LIST
{
	pthread_mutex_t 		m_lock;
	list<_SEND_T> 			m_list;

	_SEND_LIST() { pthread_mutex_init(&m_lock, NULL); }
	~_SEND_LIST(){ pthread_mutex_destroy(&m_lock); }

	void lock() { pthread_mutex_lock(&m_lock); }

	void unlock() { pthread_mutex_unlock(&m_lock); }
	
}SEND_LIST;


static SEND_LIST s_send_queue;		// TODO:优化，拒绝全局变量


int QSend_push(int socket, void* data, int len)
{
	_SEND_T x;
	s_send_queue.lock();

	x.target_socket = socket;
	x.data.assign((char*)data, len);

	if (QSend_Socket_of_front() == socket)
	{
		s_send_queue.m_list.begin()->data += x.data;	
	}
	else
	{
		s_send_queue.m_list.push_back(x);
	}
	
	s_send_queue.unlock();

	return 0;
}

int QSend_pop_first(int socket, void* data, int* len)
{
	int ret = 0;

	s_send_queue.lock();

	auto it = find_if(s_send_queue.m_list.begin(), s_send_queue.m_list.end(), [&](const _SEND_T& element)->bool{ return socket == element.target_socket;});
	if (it == s_send_queue.m_list.end())
	{
		s_send_queue.unlock();
		return -1;
	}
	
	if (*len < it->data.size())
	{
		ret = -1;
	}
	else
	{
		memcpy(data, it->data.c_str(), it->data.size());
		ret = 0;
	}
	*len = it->data.size();
	s_send_queue.m_list.erase(it);
	
	s_send_queue.unlock();
	
	return ret;
}

int QSend_Socket_of_front()
{
	int socket = -1;
	s_send_queue.lock();

	auto it = s_send_queue.m_list.begin();
	if (it != s_send_queue.m_list.end())
	{
		socket = it->target_socket;
	}
	s_send_queue.unlock();

	return socket;
}

int QSend_size()
{
	int size = 0;
	s_send_queue.lock();
	size = s_send_queue.m_list.size();
	s_send_queue.unlock();

	return size;
}










