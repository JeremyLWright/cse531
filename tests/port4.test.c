typedef int message_value_type;
#define message_size        10
#define PORT_DEPTH          10
#define NUM_PORTS           100 //Final requirement is 100
#include "msgs.h"
#include <assert.h>
#include <unistd.h>

Port_t ports[NUM_PORTS];

void server(void)
{
    int j = 0;
    const size_t my_tid = tid(CurrQ(&RunQ));
    while(1)
    {
        printf("I am the server (%lu)!\n", my_tid);
        for(j = 0; j < NUM_PORTS; ++j)
        {
            message_t response = Receive(&ports[j]);
            response.payload[1]++;
            Send(&ports[j], response);
        }
    }
}

void client(void)
{
    int i = 0;
    const size_t my_tid = tid(CurrQ(&RunQ));
    int payload[message_size];
    memset(payload, 0, message_size);
    payload[0] = my_tid;

    while(1)
    {
        payload[1] = i++;
        printf("[%lu] Sending: %d\n", my_tid, payload[1]);
        message_t msg = make_message(payload, message_size);
        Send(&ports[my_tid-1], msg);
        message_t recvd = Receive(&ports[my_tid-1]);
        printf("[%lu] Received: %d\n", my_tid, recvd.payload[1]);
        usleep(100);
    }
}


int main(int argc, const char *argv[])
{
    int i;
    printf("This should be stable.");

    InitQ(&RunQ);
    for(i = 0; i < NUM_PORTS; ++i)
        PortInit(&ports[i], PORT_DEPTH);
    
    for(i = 0; i < NUM_PORTS; ++i)
        start_thread(client);

    start_thread(server);
    run();
    return 0;
}
