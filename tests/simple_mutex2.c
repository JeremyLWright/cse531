#include "sem.h"
#include "unistd.h"

semaphore_t mutex;

size_t i=0;
void f1()
{
    while(1)
    {
        sleep(1);
        P(&mutex);
        ++i;
        V(&mutex);
    }
}

void f2()
{
    while(1)
    {
        sleep(1);
        P(&mutex);
        printf("\ti: %lu\n", i);
        V(&mutex);
    }
}

int main(int argc, const char *argv[])
{
    init_sem(&mutex, 1);
    start_thread(f1);
    start_thread(f1);
    start_thread(f1);
    start_thread(f1);
    start_thread(f1);
    start_thread(f1);
    start_thread(f2);
    run();
    
    return 0;
}
