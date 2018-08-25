
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <map>
#include <string>
#include <algorithm>

#include <pthread.h>

#include "recvhelper.h"
#include "config.h"



using namespace std;


typedef int CLTSOCKET;
typedef string DATA;


typedef struct recv_helper_t
{
	pthread_mutex_t 		m_lock;
	map<CLTSOCKET, DATA> 	m_datamap;

	recv_helper_t() { pthread_mutex_init(&m_lock, NULL); }
	~recv_helper_t(){ pthread_mutex_destroy(&m_lock); }

	void lock() { pthread_mutex_lock(&m_lock); }

	void unlock() { pthread_mutex_unlock(&m_lock); }
	
}recv_helper_t;



recv_helper_t* create_recv_helper()
{
	return new recv_helper_t();
}


void release_recv_helper(recv_helper_t* helper)
{
	delete helper;
	return;
}

int MRecv_insert(recv_helper_t* helper, int client_socket, const void* data, int len)
{
	helper->lock();

	if (helper->m_datamap.end() != helper->m_datamap.find(client_socket))
	{
		assert(0);
		helper->unlock();
		return -1;
	}
		

	helper->m_datamap[client_socket] = string().assign((char*)data, len);

	helper->unlock();

	return 0;
}


int MRecv_find(recv_helper_t* helper, int client_socket, void* data, int* len)
{
	int ret = 0;
	helper->lock();

	auto it = helper->m_datamap.find(client_socket);
	if (it == helper->m_datamap.end())
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
	
	helper->unlock();

	return ret;
}


int MRecv_remove(recv_helper_t* helper, int client_socket)
{
	int ret = 0;
	
	helper->lock();

	auto it = helper->m_datamap.find(client_socket);
	if (it == helper->m_datamap.end())
	{
		// do nothing
	}
	else
	{
		helper->m_datamap.erase(it);
	}

	helper->unlock();

	return 0;
}











