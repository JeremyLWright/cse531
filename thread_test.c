#include "threads.h"
#include "unistd.h" // usleep()
#include <stdio.h>

static int globalInt1 = 42;
static int globalInt2 = 1024;
static const int sleep_usec = 250;

// TODO: define more interesting functions that may be passed to start_thread()
void function1(){
    static int i = 10;
    while(1) {
        i++;
        usleep(sleep_usec);
        printf("Function1 waking up for the %dth time - Global %d...\n", i, globalInt1);
        yield();
    }
}

void function2(){
    static int i = 0;
    while(1) {
        i++;
        usleep(sleep_usec);
        printf("Function2 is awake and computing (%d times) - Global %d...\n", i, globalInt2);
        yield();
    }
}


int main(int argc, const char *argv[])
{

    int i = 0;
    // allocate a RunQ
    InitQ(&RunQ);
    printf("Begin Main thread\n");

    // Let's see Linux Kernel try to manage 1M threads! :-P
    // Yeah, 13,105,680 KB of memory
    for(i = 0; i < 1000000; ++i) 
    {
        start_thread(function1);    
        start_thread(function2);
    }
    run(); //I'm pretty sure this never returns

    while (1) //I'm pretty sure this never runs
    {
        usleep(sleep_usec);
    }
    return 0;

}


