
#pragma once

/*
	@bref 双向列表
*   @data 2018-04-27
*   @author guok
*   @remark 需要使用者自己管理data的内存，销毁list的时候需要提供释放内存的回调函数
*/

typedef void (*datafree)(void* data);
typedef struct easy_list_t easy_list_t;
typedef struct easy_item_t easy_item_t;

#ifdef __cplusplus
extern "C" {
#endif

int easy_list_init(easy_list_t** list, datafree func);

int easy_list_uninit(easy_list_t** list);

int easy_list_insert_to_head(easy_list_t* list, void* data);
int easy_list_insert_to_tail(easy_list_t* list, void* data);

easy_item_t* easy_list_head(easy_list_t* list);
easy_item_t* easy_list_tail(easy_list_t* list);

/*
*	pos is bool value. if set to zero, new element will be add to the position where behind of target item
*/
int easy_list_add(easy_list_t* list, easy_item_t* item, int pos, void* data);

int easy_list_remove(easy_list_t* list, easy_item_t* item);

int easy_list_setData(easy_item_t* item, void* newdata, void** olddata);
int easy_list_getData(easy_item_t* item, void** data);

easy_item_t* easy_list_next(easy_item_t* item);

easy_item_t* easy_list_prev(easy_item_t* item);

int easy_list_size(easy_list_t* list);


#ifdef __cplusplus
}
#endif

