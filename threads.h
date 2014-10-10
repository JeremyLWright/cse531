#pragma once

#ifndef LIST_PARAM
    typedef TCB_t list_parameter_t;
    #define LIST_PARAM
#endif
	
#include "q.h"

Q RunQ; // TODO: need to malloc this q - main??

void start_thread(void (*function)(void))
{ // begin pseudo code
    //allocate a stack (via malloc) of a certain size (choose 8192)
    const int stack_size = 8192;
    void * stackP;
    stackP = (void*) malloc(stack_size * sizeof(Byte) );
      
    //allocate a TCB (via malloc)
    TCB_t * newTCB;
    newTCB = (TCB_T*) malloc(sizeof(TCB_t));

    //call init_TCB with appropriate arguments
    init_TCB(&newTCB, function, &stackP, stack_size);

    //call addQ to add this TCB into the “RunQ” which is a global header pointer
    // TODO: where does the q come from?  is it assumed global in the main context?
    AddQ(RunQ, &newTCB);
  //end pseudo code
}

void run()
{   // real code
    ucontext_t parent;     // get a place to store the main context, for faking

    // int getcontext(ucontext_t *ucp); 
    // initializes the structure pointed at by ucp to the currently active context. 
    getcontext(&parent);   // magic sauce

    // TODO: Is this supposed to be RunQ->context or RunQ->next ??
    swapcontext(&parent, RotateQ(q)->context);  // start the first thread
    // swapcontext will return (0) only when the current context is reactivated
}

void yield() // similar to run
{
    //rotate the run Q;
	list_parameter_t* current = Peek(q);
    //swap the context, from previous thread to the thread pointed to by runQ
    // int swapcontext(ucontext_t *oucp, ucontext_t *ucp); saves the current context in
    // the structure pointed to by oucp, and then activates the context pointed to by ucp.
    swapcontext(current->context, RotateQ(q)->context);
    // swapcontext will return 0 when the current (yielding) context is reactivated
}
