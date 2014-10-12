#pragma once
#include <strings.h>
/*----------------------- q.h --------------------------------------------*/
#ifndef LIST_PARAM
typedef struct _test_item_t {
    int data;
    struct _test_item_t* next;
    struct _test_item_t* prev;
} test_item_t;
typedef test_item_t list_parameter_t;
    #define LIST_PARAM
#endif
// Head    read            write                 Tail
// V       V -> reads ->   V  -> writes ->        V   
// [ -------------- ---------- --------- -------- ]
typedef struct _Q {
    list_parameter_t* head;
    list_parameter_t* tail;
    list_parameter_t* curr;
    size_t size;
} Q;

void InitQ(Q* q)
{
    bzero(q, sizeof(Q));
}
size_t size_(Q* q)
{
    return q->size;
}
void AddQ(Q* q, list_parameter_t * item)
{
    if(q->head == 0)
    {
        item->prev = item;
        item->next = item;
        q->head = item;
    }
    else
    {
        item->prev = q->tail;
        item->next = q->head;
        q->tail->next = item;
    }
    q->tail = item;
    q->curr = item;
    ++q->size;
}

list_parameter_t* DelQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q->head == 0)
    {   
        return 0;
    }
    list_parameter_t* r = q->tail;
    q->curr = q->tail;
    q->tail = q->tail->prev;
    --q->size;
    return r;
}
list_parameter_t* RotateQ(Q* q)
{
    q->curr = q->curr->next;
    return q->curr;
}
