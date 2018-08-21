

#pragma once

typedef struct es_svrinfo_t es_svrinfo_t;

typedef int (*task_cb)(const void* in_data, int in_len, void* out_data, int* out_len);
typedef void(*after_work_cb)(void* param, int status);
typedef void(*cancel_works_cb)(void* param);


#ifdef __cplusplus
extern "C" {
#endif


	es_svrinfo_t* create_svrNet();

	int svrNet_setThreadPool(es_svrinfo_t* sn, task_cb w, after_work_cb aw, cancel_works_cb cw, int thread_num);

    /*
    *   开启服务器端网络模块
    *   [参数]  ip:   服务器端绑定的IP
                port:  服务器端使用的端口
    */
    int svrNet_start(es_svrinfo_t* sn, char* ip, short port);


    void svrNet_stop(es_svrinfo_t* info);


	void Destroy_svrNet(es_svrinfo_t* info);

#ifdef __cplusplus
}
#endif


