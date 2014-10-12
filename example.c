#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _TCB_t {
    ucontext_t ctx;
} TCB_t;
#define QUEUE_SIZE  3
TCB_t ctx[QUEUE_SIZE];
size_t idx = 0;

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void yield()
{
    size_t current = idx;
    idx = (idx + 1) % QUEUE_SIZE;
    size_t next = idx;
    printf("Status: (%lu, %lu)", current, next);
    swapcontext(&ctx[current].ctx, &ctx[next].ctx);
    printf("\tHello from yield.\n");
}

void
func1(void)
{
    int i =0;
    printf("func1: started\n");
    while(1)
    {
        printf("[%d] ", ++i);
    printf("func1: swapcontext(&uctx_func1, &uctx_func2)\n");
    yield();
    printf("func1: returning\n");
    }
}
    
void
func2(void)
{
    int j = 10;
    printf("func2: started\n");
    while(1)
    {
        printf("[%d] ", ++j);
    printf("func2: swapcontext(&uctx_func2, &uctx_func1)\n");
    yield();
    printf("func2: returning\n");
    }
}

    void
func3(void)
{
    int j = 10;
    printf("func3: started\n");
    while(1)
    {
        printf("[%d] ", ++j);
    printf("func3: swapcontext(&uctx_func3, &uctx_func1)\n");
    yield();
    printf("func3: returning\n");
    }
}
void start_thread(TCB_t* temp, void (*function)(void))
{ // begin pseudo code
    //allocate a stack (via malloc) of a certain size (choose 8192)
    int const  stack_size = 8192;
    getcontext(&temp->ctx);
    temp->ctx.uc_stack.ss_sp = malloc(stack_size * sizeof(char) );
    temp->ctx.uc_stack.ss_size = stack_size;
    //temp->ctx.uc_link = &uctx_main;
    makecontext(&temp->ctx, function, 0);
}

void run()
{   // real code
    ucontext_t parent;     // get a place to store the main context, for faking

    //// int getcontext(ucontext_t *ucp); 
    //// initializes the structure pointed at by ucp to the currently active context. 
    getcontext(&parent);   // magic sauce

    //swapcontext(&parent, RotateQ(&RunQ)->context);  // start the first thread
    // swapcontext will return (0) only when the current context is reactivated
   swapcontext(&parent, &ctx[0].ctx);
}


int
main(int argc, char *argv[])
{
    start_thread(&ctx[0], func1);
    start_thread(&ctx[1], func2);
    start_thread(&ctx[2], func3);


   printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
   run();

   printf("main: exiting\n");
    exit(EXIT_SUCCESS);
}
