#pragma once

#include "TCB.h"

#ifndef LIST_PARAM
    typedef TCB_t list_parameter_t;
    #define LIST_PARAM
#endif
	
#include "q.h"
#include <stdint.h>

Q RunQ;

void start_thread(void (*function)(void))
{ // begin pseudo code
    //allocate a stack (via malloc) of a certain size (choose 8192)
    int const  stack_size = 8192;
    void * stackP = malloc(stack_size * sizeof(uint8_t) );
      
    //allocate a TCB (via malloc)
    TCB_t * newTCB = (TCB_t*) malloc(sizeof(TCB_t));

    //call init_TCB with appropriate arguments
    init_TCB(newTCB, function, stackP, stack_size);

    //call addQ to add this TCB into the “RunQ” which is a global header pointer
    AddQ(&RunQ, newTCB);
  //end pseudo code
}

void run()
{   // real code
    ucontext_t parent;     // get a place to store the main context, for faking

    // int getcontext(ucontext_t *ucp); 
    // initializes the structure pointed at by ucp to the currently active context. 
    getcontext(&parent);   // magic sauce

    swapcontext(&parent, &(RotateQ(&RunQ)->context));  // start the first thread
    // swapcontext will return (0) only when the current context is reactivated
}

void yield() // similar to run
{
    //rotate the run Q;
	list_parameter_t* current = PeekQ(&RunQ);
    //swap the context, from previous thread to the thread pointed to by runQ
    // int swapcontext(ucontext_t *oucp, ucontext_t *ucp); saves the current context in
    // the structure pointed to by oucp, and then activates the context pointed to by ucp.
    swapcontext(&(current->context), &(RotateQ(&RunQ)->context));
    // swapcontext will return 0 when the current (yielding) context is reactivated
}
