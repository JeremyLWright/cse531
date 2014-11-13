
typedef int message_value_type;
#define message_size        10
#include "msgs.h"
#include <assert.h>
#include <unistd.h>

Port_t portA;

void fn(void)
{
    int i, j = 0;
    int payload1a[] = {1,2,3,4};
    int payload2a[] = {5,6,7,8};
    while(1)
    {
        message_t msg1a = make_message(payload1a, 4);
        message_t msg2a = make_message(payload2a, 4);

        Send(&portA, msg1a);
        Send(&portA, msg2a);

        message_t msg1b = Receive(&portA);
        message_t msg2b = Receive(&portA);

        for(i = 0; i < sizeof(payload1a)/sizeof(message_value_type); ++i)
            assert(msg1a.payload[i] == msg1b.payload[i]);

        for(i = 0; i < sizeof(payload2a)/sizeof(message_value_type); ++i)
            assert(msg2a.payload[i] == msg2b.payload[i]);
        sleep(1);
        printf("Run: %d\n", ++j);
    }
}
int main(int argc, const char *argv[])
{
    InitQ(&RunQ);
    PortInit(&portA, 10);
    start_thread(fn);
    run();
    return 0;
}
