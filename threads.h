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
{   // real code
    ucontext_t parent;     // get a place to store the main context, for faking

    //// int getcontext(ucontext_t *ucp); 
    //// initializes the structure pointed at by ucp to the currently active context. 
    getcontext(&parent);   // magic sauce

    //swapcontext(&parent, RotateQ(&RunQ)->context);  // start the first thread
    RotateQ(&RunQ); //TODO, try commenting and removing this one. It changes the behavior weirdly
    // swapcontext will return (0) only when the current context is reactivated
setcontext(&RunQ.curr->ctx);
//   swapcontext(&parent, &RunQ.curr->ctx);
}

void yield()
{
    RotateQ(&RunQ);
    swapcontext(&RunQ.curr->ctx, &RunQ.curr->next->ctx);
    printf("\tHello from yield.\n");
}

