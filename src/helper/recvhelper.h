#pragma once


#include "conn.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct recv_helper_t recv_helper_t;


recv_helper_t* create_recv_helper();

int MRecv_insert(recv_helper_t* helper, CONNID id, const void* data, int len);
int MRecv_find(recv_helper_t* helper, CONNID id, void* data, int* len);
int MRecv_remove(recv_helper_t* helper, CONNID id);

void release_recv_helper(recv_helper_t* helper);

#ifdef __cplusplus
}
#endif
