typedef int message_value_type;
#define message_size        10  // For simplicity, a message is an array of 10 integers.
#define PORT_DEPTH          10  // make N=10 (num  msgs in buffer)
// Declare a set (array of ports). The ports are numbered 0 to 99.
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
            // the server should receive from the client then 
            message_t response = Receive(&ports[j]);
            // do some work
            response.payload[1]++;
            // send back to the client
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
        // the client should send to the server
        Send(&ports[my_tid-1], msg);
        // do NOTHING until the server returns the message
        // wait on a receive on some port
        message_t recvd = Receive(&ports[my_tid-1]);
        printf("[%lu] Received: %d\n", my_tid, recvd.payload[1]);
        usleep(100);
    }
}


int main(int argc, const char *argv[])
{
    int i;
    printf("This should be stable.\n");

    InitQ(&RunQ);
    // Declare a set (array of ports). The ports are numbered 0 to 99.
    for(i = 0; i < NUM_PORTS; ++i)
        PortInit(&ports[i], PORT_DEPTH);
    
    for(i = 0; i < NUM_PORTS; ++i)
        start_thread(client);

    start_thread(server);
    run();
    return 0;
}
