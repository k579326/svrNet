

#pragma once


/*!
*   Ë«Ïò¶ÓÁÐ
*
*   @data 2018-04-12
*   @author guok
*/
#ifdef __cplusplus
extern "C" {
#endif


typedef void (*datafree_f)(void* data);
typedef struct easy_queue_t easy_queue_t;

int easy_queue_init(struct easy_queue_t** queue);

int easy_queue_insert(struct easy_queue_t* queue, const void* data);

int easy_queue_remove(struct easy_queue_t* queue, void** data);

int easy_queue_uninit(struct easy_queue_t* queue, datafree_f func);

int easy_queue_size(struct easy_queue_t* queue);



#ifdef __cplusplus
}
#endif


