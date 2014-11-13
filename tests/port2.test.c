

typedef int message_value_type;
#define message_size        10
#include "msgs.h"
#include <assert.h>
#include <unistd.h>

Port_t portA;
int payload1a[] = {1,2,3,4};
int writer_id = 0;
int reader_id = 0;

void writer(void)
{
    int myID = ++writer_id;
    printf("Writer thread [%d] beginning\n",myID);
    int j = 0;
    while(1)
    {
        message_t msg1a = make_message(payload1a, 4);
        //printf("WRITER: payload1a[%d]=%d, msg1a[%d]=%d\n", 0, payload1a[0],0,msg1a.payload[0]);
        Send(&portA, msg1a);
        sleep(1);
        printf("Writer[%d] Run: %d\n", myID, ++j);
        printf("Successful Writes: %lu\n", portA.write_idx);
    }
}

void reader(void)
{
    int myID = ++reader_id;
    printf("\tReader thread [%d] beginning\n", myID);
    int i, j = 0;
    while(1)
    {
        message_t msg1b = Receive(&portA);
        for(i = 0; i < sizeof(payload1a)/sizeof(message_value_type); ++i){
           // printf("\tREADER[%d]: payload1a[%d]=%d, msg1b[%d]=%d\n", 
           //     myID, i, payload1a[i],i,msg1b.payload[i]);
            assert(payload1a[i] == msg1b.payload[i]);
         }
        sleep(1);
        printf("\tReader[%d] Run: %d\n", myID, ++j);
    }
}


int main(int argc, const char *argv[])
{
    printf("If there are no available readers to clear the port queue,\n\
this test will crash with a deadlock after 10 iterations.\n\
There are no readers to pull from the queue, thus the writer \n\
will fill up the queue, get pushed on the semaphore's queue, \n\
then the process will exit since there are no runnable threads.\n");

    InitQ(&RunQ);
    PortInit(&portA, 10);
    start_thread(writer);
    start_thread(writer);
    start_thread(writer);
    start_thread(writer);
    start_thread(writer);
    run();
    return 0;
}
