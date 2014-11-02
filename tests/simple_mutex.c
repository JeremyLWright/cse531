#include "sem.h"
#include "unistd.h"

semaphore_t mutex;

size_t i=0;
void do_it()
{
    P(&mutex);
    ++i;
    V(&mutex);
    while(1)
    {
        sleep(1);
        P(&mutex);
        ++i;
        printf("\ti: %lu\n", i);
        V(&mutex);
    }
}

int main(int argc, const char *argv[])
{
    InitQ(&RunQ);
    mutex = CreateSem(1);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    start_thread(do_it);
    run();
    
    return 0;
}
