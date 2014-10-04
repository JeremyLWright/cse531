#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "TCB.h"


typedef int list_parameter_t;


// - head != tail
// - curr == head, queue is empty
// - curr == tail, queue is full
typedef struct _Q {
    list_parameter_t* head = 0;
    list_parameter_t* tail = 0;
    list_parameter_t* curr_read = 0;
    list_parameter_t* curr_write = 0;
} Q;


size_t const start_size = 8;
size_t const growth_factor = 2;

size_t size_(Q* q)
{
    return (q->curr_write - q->curr_read);
}

size_t reserved_(Q* q)
{
    return (q->tail - q->head);
}

void InitQ_(Q* q, size_t init_size)
{
    q->head = (list_parameter_t*)malloc(init_size*sizeof(list_parameter_t));
    bzero(q->head, init_size);
    q->tail = (q->head)+init_size;
    q->curr_write = q->head;
    q->curr_read = q->head;
}

void InitQ (Q* q)  //Note that if Q is a head pointer to the queue, then InitQ will have to be passed &Q.
{
    InitQ_(q, start_size);
}

void AddQ(Q* q, list_parameter_t const * item)
{
    //are we full?
    if(q->curr_write == q->tail)
    {
        //We need to realloc
        size_t curr_size = size_(q);
        size_t new_size = reserved_(q)*growth_factor;
        printf("Growing...%p, %lu, %lu\n", q->head, curr_size, new_size);
        Q old_q = *q;
        InitQ_(q, new_size);
        memcpy(q->head, old_q.head, curr_size);
        q->curr_write = q->head + curr_size;
        q->curr_read = q->head + (old_q.curr_read - old_q.head);
        free(old_q.head);
    }
    memcpy(q->curr_write, item, sizeof(list_parameter_t));
    ++(q->curr_write);
}

list_parameter_t* DelQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q->curr_write == q->head)
    {
        return 0;
    }
    return --q->curr_write;
}

list_parameter_t* PeekQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q->curr_write == q->head)
    {
        return 0;
    }
    return (q->curr_write-1);
}

void FreeQ(Q* q)
{
    free(q->head);
    q->head = 0;
    q->tail = 0;
    q->curr_write = 0;
    q->curr_read = 0;
}

void RotateQ(Q* q) // deletes the head and adds it to the tail, by just moving the header pointer to the next item.
{
    AddQ(q, DelQ(q));
}

