/*******************************************************************************
 * FILENAME:    q.h
 * DESCRIPTION: TCB queue implementation for user-space threads. This queue's 
 * implementation is based on Cormen, Leiserson, Rivest, Stein, Introduction to 
 * Algorithms
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Distributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#pragma once
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/*----------------------- q.h --------------------------------------------*/
#ifndef LIST_PARAM
typedef struct _test_item_t {
    int data;
    struct _test_item_t* next;
    struct _test_item_t* prev;
} test_item_t;
typedef test_item_t list_value_type;
    #define LIST_PARAM
#endif

typedef struct _Q {
    list_value_type* nil; 
    list_value_type* head;
    list_value_type* tail;
    list_value_type* curr;
    size_t size;
} Q;

void invariants(Q* q)
{
    //assert(q->nil->tail->next == q->nil);
}

void InitQ(Q* q)
{
    memset(q, 0, sizeof(Q));
    q->nil = (list_value_type*)malloc(sizeof(list_value_type));
    q->nil->next = q->nil;
    q->nil->prev = q->nil;
    q->head = q->nil;
    q->tail = q->nil;
}
size_t size_(Q* q)
{
    invariants(q);
    return q->size;
}
void AddQ(Q* q, list_value_type * x)
{
    if(x == 0 || q == 0)
        return;
    
    invariants(q);
   
    x->next = q->nil->next;
    q->nil->next->prev = x;
    q->nil->next = x;
    x->prev = q->nil;
    
    q->size++;
    
    if(size_(q) == 1)
        q->curr = x;
    
    invariants(q);
}

list_value_type* DelQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q == 0 || q->head == 0 || q->curr == 0 || size_(q) == 0)
    {   
        return 0;
    }
    
    invariants(q);
    list_value_type* x = q->curr;

    x->prev->next = x->next;
    x->next->prev = x->prev;

    q->size--;

    invariants(q);
    return x;
}
list_value_type* RotateQ(Q* q)
{
    if(q->head == 0 || q->curr == 0)
        return 0;
    invariants(q);

    if(q->curr->next != q->nil)
        q->curr = q->curr->next;
    else
        q->curr = q->curr->next->next;

    invariants(q);
    return q->curr;
}

list_value_type* PeekQ(Q* q)
{
	return q->curr;
}
