typedef int message_value_type;
#define message_size        10  // For simplicity, a message is an array of 10 integers.
#define PORT_DEPTH          10  // make N=10 (num  msgs in buffer)
// Declare a set (array of ports). The ports are numbered 0 to 99.
#define NUM_PORTS           100 //Final requirement is 100
#define NUM_SERVERS 10
#define NUM_CLIENTS 50  // should not exceed NUM_PORTS - NUM_SERVERS
#include "msgs.h"
#include <assert.h>
#include <unistd.h>
#include <stdlib.h> // rand(), srand()
#include <time.h> // time()nistd.h>

Port_t ports[NUM_PORTS];
typedef unsigned int port_id_t;
port_id_t g_next_server = 0;
port_id_t g_next_client = NUM_SERVERS;

int randint(int max) {
    int randomNumber = (int)rand();
    int retVal = randomNumber % max;
//    printf("randomNumber = %d, retVal = %d\n", randomNumber, retVal);
    return retVal;
} 

void initrand() { srand((unsigned)(time(0))); }

int getNextServerPort(){
    port_id_t myPort = g_next_server;
    g_next_server++;
    return (myPort);
}

int getNextClientPort(){
    port_id_t myPort = g_next_client;
    g_next_client++;
    return (myPort);
}

port_id_t chooseServer(){
    port_id_t server_port = randint(NUM_SERVERS); 
    return (server_port);
}

void server(void)
{// server should listen on his own port only
    const size_t my_tid = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextServerPort();
    port_id_t destPort = -1;
    size_t clientID = 0;
    printf("I am server[%lu], listening on port #%u !\n", my_tid, myPort);
    while(1)
    {
        // server checks for messages on its own port only
        {
            // the server should receive from the client
            message_t response = Receive(&ports[myPort]);

            // get the client's ID and return port
            clientID = response.payload[0];
            destPort = response.payload[1];

            printf("Server[%lu] recv (%d,%d) on port %u from client[%lu]\n", 
                my_tid, response.payload[2], response.payload[3], myPort, clientID);

            // set header fields (tid, port) to server's info
            response.payload[0] = (message_value_type)my_tid;
            response.payload[1] = myPort;

            // add the client's data for them
            response.payload[4] = response.payload[2] + response.payload[3];
            printf("Server[%lu] send result (%d) on port %u to client[%lu]\n", 
                my_tid, response.payload[4], destPort, clientID);
            
            // send back to the client
            Send(&ports[destPort], response);
        }
    }
}

void client(void)
{
    const size_t my_tid = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextClientPort();
    port_id_t destPort = 0; 
    int payload[message_size];
    memset(payload, 0, message_size);
    int i = my_tid;
    int j = my_tid;
    // payload contents: {my_tid, myPort, myInteger,...}
    payload[0] = my_tid;
    payload[1] = myPort;

    while(1)
    {
        payload[2] = i++;
        payload[3] = j++;
        payload[4] = 0;
        destPort = chooseServer(); // randomly choose server port to send to
        printf("\tClient[%lu] send (%d,%d,%d) to port %u\n", 
            my_tid, payload[2], payload[3], payload[4], destPort);
        // build the message
        message_t msg = make_message(payload, message_size);
        // the client should send to the server 
        Send(&ports[destPort], msg);
        // do NOTHING until the server returns the message
        // wait on a receive on some port
        message_t recvd = Receive(&ports[myPort]);
        printf("\tClient[%lu] rcvd: (%d,%d,%d) from server[%d]\n", 
            my_tid, recvd.payload[2], recvd.payload[3], recvd.payload[4], recvd.payload[0]);
        //sleep(1);
        usleep(500000);
    }
}


int main(int argc, const char *argv[])
{
    int i;
    printf("This should be stable.\n");
    initrand();

    InitQ(&RunQ);
    // Declare a set (array of ports). The ports are numbered 0 to 99.
    for(i = 0; i < NUM_PORTS; ++i)
        PortInit(&ports[i], PORT_DEPTH);
    
    // start 10 servers, listening on ports 0 to 9 (the "known" ports)
    for(i = 0; i < NUM_SERVERS; ++i)
        start_thread(server);

    // start multiple clients
    for(i = NUM_SERVERS; i < (NUM_SERVERS+NUM_CLIENTS) ; ++i)
        start_thread(client);

    run();
    return 0;
}


