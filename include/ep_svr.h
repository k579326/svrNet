#pragma once

typedef struct net_svr_t net_svr_t;
typedef int (*TASKHANDLER)(const void* in_data, int in_len, void* out_data, int* out_len);

#ifdef __cplusplus
extern "C" {
#endif

net_svr_t* net_create(TASKHANDLER proc, int poolsize);

int net_start(net_svr_t* svr, char* ip, short port);

int net_destroy(net_svr_t* info);


#ifdef __cplusplus
}
#endif


