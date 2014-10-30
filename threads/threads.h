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
    list_parameter_t* tcb = (list_parameter_t*)malloc(sizeof(list_parameter_t));
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
    swapcontext(&RunQ.curr->prev->ctx, &RunQ.curr->ctx);
}

