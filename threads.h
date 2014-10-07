#pragma once
#include "q.h"
    

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
    AddQ(q, &newTCB);
  //end pseudo code
}

void run()
{   // real code
    ucontext_t parent;     // get a place to store the main context, for faking

    // TODO: where does the parentcontext get populated??  
    // is this just a dummy context that never gets used??
    getcontext(&parent);   // magic sauce

    // TODO: Is this supposed to be RunQ->context or RunQ->next ??
    swapcontext(&parent, &(RunQ->conext));  // start the first thread
}

void yield() // similar to run
{
    //rotate the run Q;
    RotateQ(q);
    //swap the context, from previous thread to the thread pointed to by runQ
    // TODO: is this the correct order?
    swapcontext($(RunQ->head), &(RunQ->head->prev));
}
