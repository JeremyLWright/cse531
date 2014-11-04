

typedef int message_value_type;
#define message_size        10
#include "msgs.h"
#include <assert.h>
#include <unistd.h>

Port_t portA;
int payload1a[] = {1,2,3,4};

void writer(void)
{
    int j = 0;
    while(1)
    {
        message_t msg1a = make_message(payload1a, 4);
        Send(&portA, msg1a);
        sleep(1);
        printf("Writer Run: %d\n", ++j);
        printf("Successful Writes: %lu\n", portA.write_idx);
    }
}

void reader(void)
{
    int i, j = 0;
    while(1)
    {
        message_t msg1b = Receive(&portA);
        for(i = 0; i < sizeof(payload1a)/sizeof(message_value_type); ++i)
            assert(payload1a[i] == msg1b.payload[i]);
        sleep(1);
        printf("Reader Run: %d\n", ++j);
    }
}


int main(int argc, const char *argv[])
{
    printf("This test should crash with a deadlock after 10 iterations.\n\
There are no readers to pull from the queue, thus the writer \n\
will fill up the queue, get pushed on the semaphore's queue, \n\
then the process will exit since there are no runnable threads.\n");

    InitQ(&RunQ);
    PortInit(&portA, 10);
    start_thread(writer);
    start_thread(writer);
    start_thread(writer);
    start_thread(writer);
    run();
    return 0;
}
