#include "threads.h"

size_t alive = 0;

void func1(void)
{
    int i =0;
    size_t my_id = alive++;
    printf("func%lu: started\n", my_id);
    while(1)
    {
        printf("[%d] ", ++i);
        printf("func%lu: swapcontext(&uctx_func1, &uctx_func2)\n", my_id);
        yield();
        printf("func%lu: returning\n", my_id);
    }
}
    
void func2(void)
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

void func3(void)
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

void func4(void)
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


int main(int argc, char *argv[])
{
    InitQ(&RunQ);
    start_thread(func1);
    start_thread(func1);
    start_thread(func1);
    start_thread(func1);
    start_thread(func1);
#if 0
    start_thread(func2);
    start_thread(func3);
    start_thread(func4);
#endif


    printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
    run();
    
    printf("main: exiting\n");
    exit(EXIT_SUCCESS);
}
