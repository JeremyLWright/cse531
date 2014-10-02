#pragma once

#include <ucontext.h>
#include <string.h>

typedef struct _TCB_t {
    void * next;
    void * prev;
    ucontext_t context;
} TCB_t;

//The queue items used for handling the threads is the TCB_t. A TCB_t is a structure that contains a next pointer, a previous pointer and a data item of the type "ucontext_t".
//
//Initializing a TCB can be done as follows (real code):

void init_TCB (TCB_t *tcb, void *function, void *stackP, int stack_size)

// arguments to init_TCB are
//   1. pointer to the function, to be executed

//   2. pointer to the thread stack

//   3. size of the stack

{

    memset(tcb, '\0', sizeof(TCB_t));       // wash, rinse

    getcontext(&tcb->context);              // have to get parent context, else snow forms on hell

    tcb->context.uc_stack.ss_sp = stackP;

    tcb->context.uc_stack.ss_size = (size_t) stack_size;

    makecontext(&tcb->context, (void(*)())function, 0);// context is now cooked

}
