
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Gqueue.h"


struct node
{
    void* data;

    struct node* prev;
    struct node* next;
};


struct easy_queue_t
{
    int size;
    struct node* queue_head;
    struct node* queue_tail;
};




int easy_queue_init(struct easy_queue_t** queue)
{
    *queue = (easy_queue_t*)malloc(sizeof(easy_queue_t));
    (*queue)->size = 0;

    struct node* p = malloc(sizeof(struct node));
    p->data = NULL;
    p->next = NULL;
    p->prev = NULL;

    (*queue)->queue_head = p;
    (*queue)->queue_tail = p;
    return 0;
}

int easy_queue_insert(struct easy_queue_t* queue, const void* data)
{
    struct node* p = malloc(sizeof(struct node));

    p->data = data;
    p->next = NULL;
    queue->queue_tail->next = p;
    p->prev = queue->queue_tail;
    queue->queue_tail = p;
    queue->size++;

    return 0;
}

int easy_queue_remove(struct easy_queue_t* queue, void** data)
{
    struct node* p = queue->queue_head->next;

    if (p == NULL)
    {// queue is empyt
        return -1;
    }

    queue->queue_head->next = p->next;
    if (p->next)
    {
        p->next->prev = queue->queue_head;
    }
    else
    {
        queue->queue_tail = queue->queue_head;
    }

    if (data)
    {
        *data = p->data;
    }
    free(p);

    queue->size--;

    return 0;
}

int easy_queue_uninit(struct easy_queue_t* queue, datafree_f func)
{
    struct node* cursor = NULL;
    void* data;

    while (easy_queue_remove(queue, &data) != -1)
    {
        if (func)
        {
            func(data);
            data = NULL;
        }
    }

    queue->size = 0;
    free(queue->queue_head);
    free(queue);
    queue = NULL;

    return 0;
}

int easy_queue_size(struct easy_queue_t* queue)
{
    return queue->size;
}