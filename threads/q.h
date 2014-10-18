/*******************************************************************************
 * FILENAME:    q.h
 * DESCRIPTION: TCB queue implementation for user-space threads
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Distributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#pragma once
#include <string.h>
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
    memset(q, 0, sizeof(Q));
}
size_t size_(Q* q)
{
    return q->size;
}
void AddQ(Q* q, list_parameter_t * item)
{
    if(item == 0 || q == 0)
        return;

    if(q->head == 0)
    {
        item->prev = item;
        item->next = item;
        q->head = item;
    }
    else
    {
        
        q->head->prev = item;
        q->tail->next = item;
        item->prev = q->tail;
        item->next = q->head;

    }
    q->tail = item;
    q->curr = item;
    ++q->size;
}

list_parameter_t* DelQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q->head == 0 || q->curr == 0 || q->tail == 0)
    {   
        return 0;
    }
    list_parameter_t* r = q->curr;
    list_parameter_t* prev = q->curr->prev;
    list_parameter_t* next = q->curr->next;
    prev->next = next;
    next->prev = prev;
    q->curr = next;
    --q->size;
    return r;
}
list_parameter_t* RotateQ(Q* q)
{
    if(q->head == 0 || q->curr == 0)
        return 0;
    q->curr = q->curr->next;
    return q->curr;
}

list_parameter_t* PeekQ(Q* q)
{
	return q->curr;
}
