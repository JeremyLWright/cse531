#include "threads.h"
#include "unistd.h" // usleep()
#include <stdio.h>

static int globalInt1 = 42;
static int globalInt2 = 1024;
static const int sleep_usec = 250;

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

void function2(){
    int i = 0;
    static int numCalls = 0;
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
    // allocate a RunQ
    InitQ(&RunQ);

    // Let's see Linux Kernel try to manage 1M threads! :-P
    // Yeah, 13,105,680 KB of memory
    for(i = 0; i < totalThreads; ++i) 
    {
        start_thread(function1);
        start_thread(function2);
    }
    printf("Main function: %d threads queued, ready to run:\n", i);
    run(); //I'm pretty sure this never returns

    printf("Main function entering wait loop\n");
    while (1) //I'm pretty sure this never runs
    {
        usleep(sleep_usec);
    }
    return 0;

}


