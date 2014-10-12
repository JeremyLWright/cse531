#include <vector>

    
#include "unistd.h" // usleep()
#include <boost/circular_buffer.hpp>
#include <stdio.h>
#include <ucontext.h>
#include <string.h>

typedef struct _TCB_t {
    ucontext_t context;
} TCB_t;

static int globalInt1 = 42;
static int globalInt2 = 1024;
static const int sleep_usec = 250;

boost::circular_buffer<TCB_t*> RunQ(2);
boost::circular_buffer<TCB_t*>::iterator it;
ucontext_t parent;     


//The queue items used for handling the threads is the TCB_t. A TCB_t is a structure that contains a next pointer, a previous pointer and a data item of the type "ucontext_t".
//
//Initializing a TCB can be done as follows (real code):

void init_TCB (TCB_t *tcb, void (*function)(void), void *stackP, int stack_size)
{

    bzero(tcb, sizeof(TCB_t));
    getcontext(&tcb->context);              // have to get parent context, else snow forms on hell
    tcb->context.uc_stack.ss_sp = stackP;
    tcb->context.uc_stack.ss_size = (size_t) stack_size;
    makecontext(&tcb->context, (void(*)())function, 0);// context is now cooked
}
void start_thread(void (*function)(void))
{ 
    int const  stack_size = SIGSTKSZ;
    void * stackP = malloc(stack_size * sizeof(uint8_t) );
    TCB_t * newTCB = (TCB_t*) malloc(sizeof(TCB_t));
    init_TCB(newTCB, function, stackP, stack_size);
    RunQ.push_back(newTCB);
}

void run()
{   
    getcontext(&parent);
    auto current = *it;
    current->context.uc_link = &parent;
    swapcontext(&parent, &current->context);
}

void yield() // similar to run
{
	auto* current = *it;
    auto* next = *(++it);
    swapcontext(&(current->context), &(next->context));
}

// TODO: define more interesting functions that may be passed to start_thread()
void function1(){
    int i = 10;
    static int numCalls = 0;
    ++numCalls;
    while(1) {
        i++;
        usleep(sleep_usec);
        if (numCalls%100 == 0) {
        printf("Function1 (instance %d) waking up for the %dth time - Global %d...\n", 
            numCalls, i, globalInt1);
        }
        yield();
    }
}

void f3()
{
    printf("3 Hello.\n");
    yield();
    printf("3 Bye.");
}
void f4()
{
    printf("4 Hello.\n");
    yield();
    printf("4 Bye.");
}

void function2(){
    int i = 0;
    int numCalls = 0;
    ++numCalls;
    while(1) {
        i++;
        usleep(sleep_usec);
        if (numCalls%100 == 0) {
            printf("Function2 (instance %d) is awake and computing (%d times) - Global %d...\n", 
                    numCalls, i, globalInt2);
        }
        yield();
        // it seems that we're never returning from the yield and spawning new threads instead
        globalInt2++;
        printf("Function2 (instance %d) is back in context (%d times) - Global %d...\n", 
            numCalls, i, globalInt2);
    }
}


int main(int argc, const char *argv[])
{

    int i = 0;
    int totalThreads = 250000;
    if (argc > 1){
        totalThreads = atoi(argv[1]);
    }
    printf("Begin Main function\n");
    printf("Main function: spawning %d child threads...\n", totalThreads);

    // Let's see Linux Kernel try to manage 1M threads! :-P
    // Yeah, 13,105,680 KB of memory
    for(i = 0; i < totalThreads; ++i) 
    {
        //start_thread(function1);
        start_thread(f3);
        start_thread(f4);
    //    start_thread(function2);
    }
    printf("Main function: %d threads queued, ready to run:\n", i);
    it = std::begin(RunQ);
    run(); //I'm pretty sure this never returns

    printf("Main function entering wait loop\n");
    while (1) //I'm pretty sure this never runs
    {
        usleep(sleep_usec);
    }
    return 0;

}


