#pragma once

#include "conn.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct send_queue_t     send_queue_t;
typedef struct net_svr_t		net_svr_t;


typedef struct send_thread_t
{
	net_svr_t*		svr;
    pthread_t 		id;
	pthread_mutex_t lock;
	pthread_cond_t 	cond;
    send_queue_t*   cache;
}send_thread_t;



typedef int (*STARTFUNC)(send_thread_t* param);

typedef struct send_helper_t
{
	send_queue_t*	m_queue; 
    send_thread_t*  thread;
    STARTFUNC       start;
}send_helper_t;

send_helper_t* create_send_helper(const net_svr_t* svr);
// int sendhelper_start(send_helper_t* helper);

int QSend_push(send_queue_t* queue, CONNID connectId, void* data, int len);
CONNID QSend_connid_of_front(send_queue_t* queue);
int QSend_pop_first(send_queue_t* queue, CONNID connectId, void* data, int* len);
int QSend_remove_all_by_connid(send_queue_t* queue, CONNID connid);
int QSend_size(send_queue_t* queue);

void send_wait(send_thread_t* thread);
void send_signal(send_thread_t* thread);

int release_send_helper(send_helper_t* helper);






#ifdef __cplusplus
}
#endif
