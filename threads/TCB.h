/*******************************************************************************
 * FILENAME:    TCB.h
 * DESCRIPTION: Thread Control Block implementation for user-space threads
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Distributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#pragma once
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

typedef struct _TCB_t {
    ucontext_t ctx;
    struct _TCB_t* next;
    struct _TCB_t* prev;
} TCB_t;
#ifndef LIST_PARAM
    #define LIST_PARAM
    typedef TCB_t list_parameter_t;
#endif

#include "q.h"

void init_TCB (TCB_t *tcb, void *function, void *stackP, int stack_size)
{
    getcontext(&tcb->ctx);
    tcb->ctx.uc_stack.ss_sp = stackP;
    tcb->ctx.uc_stack.ss_size = stack_size;
    makecontext(&tcb->ctx, function, 0);
}
