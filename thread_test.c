#include "threads.h"


static int globalInt1 = 42;
static int globalInt2 = 1024;
// TODO: define functions that may be passed to start_thread()
void function1(){
    int i = 0;
    while(1) {
        i++;
        printf("Function1 waking up for the %dth time\n", i);
        yield();
    }
}

void function2(){
    int i = 0;
    while(1) {
        i++;
        printf("Function2 is awake and computing (%d times)\n", i);
        yield();
    }
}


int main(int argc, const char *argv[])
{

    printf("Begin Main thread\n");
    start_thread(function1);    
    start_thread(function2);
    run();

    while (1){};
    return 0;

}


