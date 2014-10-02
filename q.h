#pragma once
#include <stdbool.h>
#include "TCB.h"

typedef int list_parameter_t;


// - head != tail
// - curr == head, queue is empty
// - curr == tail, queue is full
typedef struct _Q {
    list_parameter_t* head = 0;
    list_parameter_t* tail = 0;
    list_parameter_t* curr = 0;
} Q;


size_t const start_size = 16;
size_t const growth_factor = 1.5;

size_t _size(Q* q)
{
    return (q.curr - q.head)/sizeof(list_parameter_t);
}

size_t _reserved(Q* q)
{
    return (q.tail - q.head)/sizeof(list_parameter_t);
}

void InitQ (Q* q)  //Note that if Q is a head pointer to the queue, then InitQ will have to be passed &Q.
{
    q.head = (list_parameter_t*)malloc(sizeof(list_parameter_t)*start_size); //Brilliant API deisgn. How can I propogate an errror?!?!!?
    q.tail = (q.head)+sizeof(list_parameter_t)*start_size;
    q.curr = q.head;
}

void AddQ(Q* q, int* item)
{
    //are we full?
    if(curr == tail)
    {
        //We need to realloc
        size_t curr_offset = q.curr - q.head;
        size_t new_size = _size(q)*growth_factor;
        q.head = (list_parameter_t*)realloc(q.head, new_size);
        q.tail = q.head + sizeof(list_parameter_t)*new_size;
        q.curr = q.head + curr_offset;
    }
    ++q.curr;
    q.curr = item;
}

Item* DelQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q.curr == q.head)
    {
        return 0;
    }
    return q.curr--;
}

void RotateQ(Q* q) // deletes the head and adds it to the tail, by just moving the header pointer to the next item.
{
    AddQ(q, DelQ(q));
}

