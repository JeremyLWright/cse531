#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#ifndef LIST_PARAM
    #define LIST_PARAM
    typedef int list_parameter_t;
#endif
// Head    read            write                 Tail
// V       V -> reads ->   V  -> writes ->        V   
// [ -------------- ---------- --------- -------- ]
typedef struct _Q {
    list_parameter_t* head;
    list_parameter_t* tail;
    list_parameter_t* curr_read;
    list_parameter_t* curr_write;
    size_t size;
} Q;

size_t const start_size = 8;
size_t const growth_factor = 2;
size_t size_(Q const * q)
{
    return (q->size);
}

size_t reserved_(Q const* q)
{
    return (q->tail - q->head);
}
void print_state_(Q const * q, char const * tag)
{
#if 0
    printf("%s\t{head = %p, tail = %p, curr_read = %p, curr_write = %p, size = %lu}\n",
            tag,
            q->head,
            q->tail,
            q->curr_read,
            q->curr_write,
            q->size);
    printf("{");
    for(size_t i = 0; i < reserved_(q); ++i)
    {
        printf("%d,", q->head[i]);
    }
    printf("}\n");
#endif
}


void InitQ_(Q* q, size_t init_size)
{
    q->head = (list_parameter_t*)malloc(init_size*sizeof(list_parameter_t));
    bzero(q->head, init_size*sizeof(list_parameter_t));
    q->tail = (q->head)+init_size;
    q->curr_write = q->head;
    q->curr_read = q->head;
    q->size = 0;
}

void InitQ (Q* q)  //Note that if Q is a head pointer to the queue, then InitQ will have to be passed &Q.
{
    InitQ_(q, start_size);
}

void compact_(Q* q, Q* old_q)
{
    // Case: head - read - write - tail;
    if(old_q->curr_read < old_q->curr_write)
    {
        memcpy(q->head, old_q->curr_read, (old_q->curr_write - old_q->curr_read)*sizeof(list_parameter_t));
    }
    // Case: head - write - read - tail;
    else
    {
        size_t read_to_tail = (old_q->tail - old_q->curr_read);
        size_t head_to_read = (old_q->curr_read - old_q->head);
        memcpy(q->head, old_q->curr_read, read_to_tail*sizeof(list_parameter_t));
        memcpy(q->head+read_to_tail, old_q->head, head_to_read*sizeof(list_parameter_t));
    }
    q->curr_write = q->head + old_q->size;
    q->size = old_q->size;
    free(old_q->head);
}

void AddQ(Q* q, list_parameter_t const * item)
{
    //are we at the end?
    if(q->curr_write == q->tail)
    {
        q->curr_write = q->head;
    }

    //are we full?
    if(q->size == reserved_(q))
    {
        //We need to realloc
        size_t new_size = reserved_(q)*growth_factor;
        Q old_q = *q;
        //printf("Growing...%p, %lu, %lu\n", q->head, old_q.size, new_size);
        InitQ_(q, new_size);
        compact_(q, &old_q);
    }
    memcpy(q->curr_write, item, sizeof(list_parameter_t));
    ++q->curr_write;
    ++q->size;
    print_state_(q, "Add");
    // TODO: don't we need to change prev/next pointers somewhere in here?  
}

list_parameter_t* DelQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q->size == 0)
    {   
        //printf("Zero.\n");
        return 0;
    }
    if(q->curr_read == q->tail)
    {
        q->curr_read = q->head;
    }
    --q->size;
    print_state_(q, "Del");
    return q->curr_read++;
}

list_parameter_t* PeekQ(Q* q) // will return a pointer to the item deleted.
{
    return q->curr_read;
}

void FreeQ(Q* q)
{
    free(q->head);
    q->head = 0;
    q->tail = 0;
    q->curr_write = 0;
    q->curr_read = 0;
}

list_parameter_t* RotateQ(Q* q) // deletes the head and adds it to the tail, by just moving the header pointer to the next item.
{
    if(q->size == 0)
        return 0;
    list_parameter_t* r = PeekQ(q);
    AddQ(q, DelQ(q));
	return r;
}
