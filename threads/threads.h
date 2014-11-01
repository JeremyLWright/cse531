/*******************************************************************************
 * FILENAME:    threads.h
 * DESCRIPTION: Threads implementation for user-space threads
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Distributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#pragma once
#include "TCB.h"
Q RunQ;

void start_thread(void (*function)(void))
{ // begin pseudo code
    //allocate a stack (via malloc) of a certain size (choose 8192)
    int const  stack_size = 8192;
    void* stackP = malloc(stack_size*sizeof(char));
    list_value_type* tcb = (list_value_type*)malloc(sizeof(list_value_type));
    init_TCB (tcb, function, stackP, stack_size);
    AddQ(&RunQ, tcb);
}

void run()
{   
    ucontext_t parent;   // get a place to store the main context
    getcontext(&parent);
    swapcontext(&parent, &RunQ.curr->ctx);
}

void yield()
{
    RotateQ(&RunQ); 
#ifdef DEBUG
    printf(ANSI_COLOR_MAGENTA "%s:%d From T%lu to T%lu Size: %lu\n" ANSI_COLOR_RESET,  __func__, __LINE__, RunQ.curr->prev->tid, RunQ.curr->tid, size_(&RunQ));
#endif
    swapcontext(&RunQ.curr->prev->ctx, &RunQ.curr->ctx);
}

