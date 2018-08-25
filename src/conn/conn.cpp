

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <map>

#include "conn.h"

using namespace std;

typedef struct conn_table_t
{
	conn_table_t(){ pthread_rwlock_init(&rwlock, NULL); }
	~conn_table_t(){ pthread_rwlock_destroy(&rwlock); }
	
	void rlock() { pthread_rwlock_rdlock(&rwlock); }
	void wlock() { pthread_rwlock_wrlock(&rwlock); }
	void unlock() { pthread_rwlock_unlock(&rwlock); }
	
	
	map<CONNID, svr_connect_t> 	conn_map;
	//map<SOCKFD, CONNID>			reverse_map;		// 反向映射，提高反向查找的速度
	pthread_rwlock_t			rwlock;
}conn_table_t;


static int ssmp_rand(void* data, int size)
{
    int remain = 0;
    int each_num;
    char* p = NULL;
	int i;
	unsigned int seed;
    
#ifdef __APPLE__
#include <sys/time.h>
    struct timeval tv;
    gettimeofday(&tv, NULL);
    seed = tv.tv_sec + rand() % 10000;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    // seed = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;  可能超出无符号整数seed最大范围，放弃
    seed = ts.tv_sec + rand() % 10000;
#endif
    
    srand(seed);
    remain = size % sizeof(int);

    p = (char*)data;
    for (i = 0; i < size / sizeof(int); i++)
    {
        each_num = rand();
        memcpy((void*)p, &each_num, sizeof(each_num));
        p += sizeof(each_num);
    }

    if (remain != 0)
    {
        each_num = rand();
        memcpy(p, &each_num, remain);
    }

    return 0;
}


CONNID Gen_ConnID()
{
	uint64_t id;

	ssmp_rand(&id, sizeof(uint64_t));
	
	return id;
}



conn_table_t* create_conn_table()
{
	return new conn_table_t();
}

// 返回连接ID
CONNID conn_insert(conn_table_t* table, const svr_connect_t* conn)
{
	table->wlock();

	CONNID id = Gen_ConnID();	

	auto it = table->conn_map.find(id);
	while (it != table->conn_map.end() || id == SVR_INVALID_CONNECT_ID)     
	{
		id = Gen_ConnID();
		it = table->conn_map.find(id);
	}	

	table->conn_map[id] = *conn;
	table->unlock();
	
	return id;
}

int conn_remove(conn_table_t* table, CONNID id)
{
	table->wlock();
	
	auto it = table->conn_map.find(id);
	if (it != table->conn_map.end())
	{
		table->conn_map.erase(it);
	}
	
	table->unlock();
	
	return 0;
}

int conn_find(conn_table_t* table, CONNID id, svr_connect_t* conn)
{
	int ret = 0;
	
	table->rlock();
	
	auto it = table->conn_map.find(id);
	if (it != table->conn_map.end())
	{
		memcpy(conn, &it->second, sizeof(svr_connect_t));
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	table->unlock();
	
	return ret;
}

int conn_get_sockfd(conn_table_t* table, CONNID id)
{
    int socket = -1;
    
    table->rlock();
    
    auto it = table->conn_map.find(id);
    if (it != table->conn_map.end())
    {
        socket = it->second.clt.client_fd;
    }
    
    table->unlock();
    
    return socket;
}


void release_conn_table(conn_table_t* table)
{
	delete table;
	return;
}














