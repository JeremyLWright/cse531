/*----------------------- TCB.h --------------------------------------------*/
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

void init_TCB (TCB_t *tcb, void *function, void *stackP, int stack_size)
{
    getcontext(&tcb->ctx);
    tcb->ctx.uc_stack.ss_sp = stackP;
    tcb->ctx.uc_stack.ss_size = stack_size;
    makecontext(&tcb->ctx, function, 0);
}
/*----------------------- q.h --------------------------------------------*/
#ifndef LIST_PARAM
    #define LIST_PARAM
    typedef int list_parameter_t;
#endif
// Head    read            write                 Tail
// V       V -> reads ->   V  -> writes ->        V   
// [ -------------- ---------- --------- -------- ]
typedef struct _Q {
    list_parameter_t* head;
    list_parameter_t* tail;
    list_parameter_t* curr;
} Q;

void InitQ(Q* q)
{
    bzero(q, sizeof(Q));
}
void AddQ(Q* q, list_parameter_t * item)
{
    if(q->head == 0)
    {
        item->prev = item;
        item->next = item;
        q->head = item;
    }
    else
    {
        item->prev = q->tail;
        item->next = q->head;
        q->tail->next = item;
    }
    q->tail = item;
    q->curr = item;
}

list_parameter_t* DelQ(Q* q) // will return a pointer to the item deleted.
{
    //Are we empty?
    if(q->head == 0)
    {   
        return 0;
    }
    list_parameter_t* r = q->tail;
    q->curr = q->tail;
    q->tail = q->tail->prev;
    return r;
}
list_parameter_t* RotateQ(Q* q)
{
    q->curr = q->curr->next;
    return q->curr;
}


/*----------------------- Threads.h --------------------------------------------*/
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
   swapcontext(&parent, &RunQ.curr->ctx);
}

void yield()
{
    RotateQ(&RunQ);
    swapcontext(&RunQ.curr->ctx, &RunQ.curr->next->ctx);
    printf("\tHello from yield.\n");
}

/*----------------------- thread_test.c --------------------------------------------*/
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
    void
func4(void)
{
    int j = 10;
    printf("func4: started\n");
    while(1)
    {
        printf("[%d] ", ++j);
    printf("func4: swapcontext(&uctx_func3, &uctx_func1)\n");
    yield();
    printf("func4: returning\n");
    }
}


    int
main(int argc, char *argv[])
{
    InitQ(&RunQ);
    start_thread(func1);
    start_thread(func2);
    start_thread(func3);
    start_thread(func4);


    printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
    run();
    
    printf("main: exiting\n");
    exit(EXIT_SUCCESS);
}
