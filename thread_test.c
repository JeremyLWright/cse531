#include "threads.h"
#include "unistd.h" // usleep()


static int globalInt1 = 42;
static int globalInt2 = 1024;
static const int sleep_usec = 100;

// TODO: define more interesting functions that may be passed to start_thread()
void function1(){
    int i = 0;
    while(1) {
        i++;
        usleep(sleep_usec)
        printf("Function1 waking up for the %dth time...\n", i);
        yield();
    }
}

void function2(){
    int i = 0;
    while(1) {
        i++;
        usleep(sleep_usec)
        printf("Function2 is awake and computing (%d times)...\n", i);
        yield();
    }
}


int main(int argc, const char *argv[])
{

    // allocate a RunQ
    Q RunQ;
    RunQ = (Q) malloc(sizeof(Q));
    printf("Begin Main thread\n");

    start_thread(function1);    
    start_thread(function2);
    run();

    while (1){usleep(sleep_usec)};
    return 0;

}


