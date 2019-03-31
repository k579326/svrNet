
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Glist.h"


#define MAGIC_NUM				0x12344321

struct easy_item_t
{
	easy_item_t* next;
	easy_item_t* prev;
	
	void* data;
};



struct easy_list_t
{
	easy_item_t* head;
	easy_item_t* tail;
    
	int size;
    
	datafree     freememory;
};


int easy_list_init(easy_list_t** list, datafree func)
{
	easy_item_t* header;
    
	assert(list);
	assert(func);
    
	*list = (easy_list_t*)malloc(sizeof(easy_list_t));
	(*list)->head = NULL;
	(*list)->tail = NULL;
	(*list)->size = 0;
	
	header = (easy_item_t*)malloc(sizeof(easy_item_t));
	header->next = NULL;
	header->prev = NULL;
	header->data = (void*)MAGIC_NUM;
	
	(*list)->head = header;
	(*list)->tail = header;
	(*list)->freememory = func;
    
	return 0;
}

int easy_list_uninit(easy_list_t** list)
{
	easy_item_t* item = NULL;
	
	while ((*list)->head->next != NULL)
	{
		item = (*list)->tail;
		(*list)->tail = (*list)->tail->prev;
		(*list)->tail->next = NULL;
		
		if ((*list)->freememory)
		{
			(*list)->freememory(item->data);
			item->data = NULL;
		}
		
		free(item);
		item = NULL;
		
		(*list)->size--;
	}
	
	assert((*list)->size == 0);
	assert((*list)->head == (*list)->tail);
	assert((*list)->head->data == (void*)MAGIC_NUM);
	
	free((*list)->head);
	free(*list);
	
	*list = NULL;
	return 0;
}


easy_item_t* easy_list_head(easy_list_t* list)
{
    return list->head->next;
}

easy_item_t* easy_list_tail(easy_list_t* list)
{
    return list->tail;
}


int easy_list_insert_to_head(easy_list_t* list, void* data)
{
    easy_item_t* item_new = (easy_item_t*)malloc(sizeof(easy_item_t));
    
    item_new->data = data;
	item_new->prev = list->head;
	item_new->next = list->head->next;
    
    if (list->head->next)
    {
        list->head->next->prev = item_new;
    }
    
    list->head->next = item_new;
    list->size++;

    return 0;
}

int easy_list_insert_to_tail(easy_list_t* list, void* data)
{
    easy_item_t* item_new = (easy_item_t*)malloc(sizeof(easy_item_t));
    
    item_new->data = data;
	item_new->prev = list->tail;
	item_new->next = NULL;
    
    list->tail->next = item_new;
    list->size++;

    return 0;
}



static int _add_to_front(easy_item_t* item, void* data)
{
	easy_item_t* item_new = (easy_item_t*)malloc(sizeof(easy_item_t));
	
	item_new->data = data;
	item_new->next = item;
	item_new->prev = item->prev;
	
	// 存在头结点，不需要判断是否空
	item->prev->next = item_new;
	item->prev = item_new;
	
	return 0;
}

static int _add_to_behind(easy_item_t* item, void* data)
{
	easy_item_t* item_new = (easy_item_t*)malloc(sizeof(easy_item_t));
	item_new->data = data;
	item_new->prev = item;
	item_new->next = item->next;

	if (item->next)
	{
		item->next->prev = item_new;
	}
	
	item->next = item_new;
	return 0;
}


int easy_list_add(easy_list_t* list, easy_item_t* item, int pos, void* data)
{
	int ret = (pos == 0 ? _add_to_behind(item, data) : _add_to_front(item, data));

    if (ret == 0)
    {
        list->size++;
    }

    return ret;
}

int easy_list_remove(easy_list_t* list, easy_item_t* item)
{
	item->prev->next = item->next;
	
	if (item->next)
	{
		item->next->prev = item->prev;
	}
	
    if (list->freememory)
    {
        list->freememory(item->data);
    }
	
	free(item);
    list->size--;

    return 0;
}

int easy_list_setData(easy_item_t* item, void* newdata, void** olddata)
{
    if (olddata)
    {
        *olddata = item->data;
    }
	
    item->data = newdata;
    return 0;
}

int easy_list_getData(easy_item_t* item, void** data)
{
    assert(data);
    *data = item->data;
    return 0;
}


easy_item_t* easy_list_next(easy_item_t* item)
{
	return item->next;
}

easy_item_t* easy_list_prev(easy_item_t* item)
{
    // 不能把无用的头结点返回去
    if (item->prev->prev = NULL)
    {
        return NULL;
    }
    
	return item->prev;
}

int easy_list_size(easy_list_t* list)
{
	return list->size;
}
