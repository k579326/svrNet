
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Glist.h"
#include "svr_net.h"
#include "ep_svr.h"


es_svrinfo_t* create_svrNet()
{
	es_svrinfo_t* info = NULL;
    
    info = (es_svrinfo_t*)malloc(sizeof(es_svrinfo_t));

	init_env(info);
	
	return info;
}

int svrNet_setThreadPool(es_svrinfo_t* info, task_cb w, after_work_cb aw, cancel_works_cb cw, int thread_num)
{
	workpool_init(&info->pool, thread_num, w, aw, cw);
	info->pool.send_thread = &info->sender;

	return 0;	
}


int svrNet_start(es_svrinfo_t* info, char* ip, short port)
{
    int err;
    struct sockaddr_in si;
   
    info->sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    
	si.sin_family = AF_INET;
	si.sin_port = htons(port);
	si.sin_addr.s_addr = inet_addr(ip);

    err = bind(info->sock_listen, (struct sockaddr*)&si, sizeof(si));
	if (err != 0)
	{
        uninit_env(info);
        free(info);
		printf("[BIND ERROR] %s\r\n", strerror(errno));
		return -1;
	}
    
	pthread_create(&info->work_tid, NULL, work_routine, info);
	pthread_create(&info->listen_tid, NULL, listen_thread, &info->sock_listen);
    
    return 0;
}

void svrNet_stop(es_svrinfo_t* info)
{
    // TODO:通知两个线程退出

	close(info->sock_listen);
	
    return;
}


void Destroy_svrNet(es_svrinfo_t* info)
{
	uninit_env(info);
	workpool_uninit(&info->pool);
	
	free(info);
	return;
}


