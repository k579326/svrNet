#pragma once

#include <stdint.h>

#define SVR_INVALID_CONNECT_ID 0xFFFFFFFFFFFFFFFF   // -1作为无效Id保留


typedef struct
{
	unsigned char		iptype;			// ipv4: 0, ipv6 1
	unsigned char		ip[16];
	int                 client_fd;
}clt_info_t;

typedef struct 
{
	clt_info_t	clt;
	int 		conntype;		// 连接类型，区分不同的网络任务
}svr_connect_t;

typedef uint64_t CONNID;
typedef struct conn_table_t conn_table_t;


#ifdef __cplusplus
extern "C" {
#endif


conn_table_t* create_conn_table();

// 返回连接ID
CONNID conn_insert(conn_table_t* table, const svr_connect_t* conn);

int conn_remove(conn_table_t* table, CONNID id);

int conn_find(conn_table_t* table, CONNID id, svr_connect_t* conn);

int conn_get_sockfd(conn_table_t* table, CONNID id);

void release_conn_table(conn_table_t* table);





#ifdef __cplusplus
}
#endif
















