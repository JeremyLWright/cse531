/*******************************************************************************
 * FILENAME:    cl-svr.c
 * DESCRIPTION: Test program for message passing system for RPC behavior
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Distributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#include <stdint.h>
#include <stdlib.h>


///////////////////////////////////////
//  Message Transport (Packets)      // 
///////////////////////////////////////

typedef unsigned int port_id_t;

typedef struct _header_t {
    port_id_t source_port;
    port_id_t dest_port;
    size_t sequence_number;
    size_t total_num_packets;
} header_t;

#define PAYLOAD_SIZE 16

typedef struct _payload_t {
    size_t len;
    char payload[PAYLOAD_SIZE];
} payload_t;

typedef struct _packet {
    header_t header;
    payload_t payload;
} packet_t;

typedef packet_t message_value_type; //Set the port to handle an array of packets
#define message_size        1   // Each message is a single packet
#define PORT_DEPTH          10  // make N=10 (num  msgs in buffer)
// Declare a set (array of ports). The ports are numbered 0 to 99.
#define NUM_PORTS           100 //Final requirement is 100
#define NUM_SERVERS 1
#define NUM_CLIENTS 90  // should not exceed NUM_PORTS - NUM_SERVERS
_Static_assert(NUM_CLIENTS <= NUM_PORTS-NUM_SERVERS, "Incorrect configuration of ports.");
#define THREAD_DELAY 90000
#include "msgs.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h> // getchar()
#include <stdlib.h> // rand(), srand()
#include <time.h> // time()nistd.h>

Port_t ports[NUM_PORTS];
port_id_t g_next_server = 0;
port_id_t g_next_client = NUM_SERVERS;

typedef struct _chunk_t {
    size_t len;
    const char* start;
} chunk_t;

chunk_t* make_chunks(
        const char* msg,
        size_t len,
        size_t* /* [out] */ number_of_chunks)
{
    size_t i;
    size_t remaining_bytes = len;
    *number_of_chunks = len % PAYLOAD_SIZE == 0 ? len/PAYLOAD_SIZE : len/PAYLOAD_SIZE + 1 ;
    chunk_t* chunks  = (chunk_t*)malloc(sizeof(chunk_t)**number_of_chunks);

    for(i = 0; i < *number_of_chunks; ++i)
    {
        chunks[i].start = msg + (i*PAYLOAD_SIZE);
        if(remaining_bytes < PAYLOAD_SIZE)
            chunks[i].len = remaining_bytes;
        else
            chunks[i].len = PAYLOAD_SIZE;
        remaining_bytes = remaining_bytes - PAYLOAD_SIZE;
    }
    return chunks;
}



packet_t* make_packet(
        const char* msg, 
        size_t len, 
        size_t source_port,
        size_t dest_port,
        size_t* /* [out] */ number_of_packets)
{
    size_t i = 0;
    chunk_t* c = make_chunks(msg, len, number_of_packets);
    packet_t* packets = (packet_t*)malloc(sizeof(packet_t)**number_of_packets);
    
    for(i = 0; i < *number_of_packets; ++i)
    {
        packets[i].header.sequence_number = i;
        packets[i].header.total_num_packets = *number_of_packets;
        packets[i].header.source_port = source_port;
        packets[i].header.dest_port = dest_port;

        packets[i].payload.len = c[i].len;
        memcpy(packets[i].payload.payload, c[i].start, c[i].len);
    }
    free(c);
    return packets;
}


///////////////////////////////////////
//   Remote Procedure Call Framework //
///////////////////////////////////////

typedef enum _command_t {
    COMMAND_ADD,
    COMMAND_DELETE,
    COMMAND_MODIFY,
    COMMAND_GET_TABLE
} command_t;

#pragma pack(1)
typedef struct _add_payload {
    const char* msg; //new message
    size_t len; //new message length
    size_t idx; //table index to modify
} add_payload_t;

typedef struct _del_payload {
    size_t idx; //table index to modify
} del_payload_t;

typedef struct _mod_payload {
    size_t idx; //table index to modify
    size_t cidx; //Character index to modify
    char c; //new character to send
} mod_payload_t;

typedef struct _get_payload {
    chunk_t string_table[10];
} get_payload_t;

typedef struct _rpc_command_t {
    command_t command;
    union  {
        add_payload_t add;
        del_payload_t del;
        mod_payload_t mod;
        get_payload_t get;
    } parameters;
} rpc_command_t;

////////////////////////
///  RPC Serializers ///
////////////////////////

packet_t* serialize_command(
        rpc_command_t command,
        port_id_t src_port,
        port_id_t dest_port,
        size_t*   /* [out] */ num_packets
        )
{
    size_t scratch_pad_size;
    char* data;
    switch(command.command)
    {
        case COMMAND_ADD:
            scratch_pad_size = sizeof(rpc_command_t)+command.parameters.add.len;
            data = (char*)malloc(scratch_pad_size);

            break;

        case COMMAND_DELETE:
            scratch_pad_size = sizeof(rpc_command_t);
            data = (char*)malloc(scratch_pad_size);
            break;

        case COMMAND_MODIFY:
            scratch_pad_size = sizeof(rpc_command_t);
            data = (char*)malloc(scratch_pad_size);
            break;

        case COMMAND_GET_TABLE:
            scratch_pad_size = sizeof(rpc_command_t)+(4096*10);
            data = (char*)malloc(scratch_pad_size);
            break;
    }

    return make_packet(data, scratch_pad_size, src_port, dest_port, num_packets);

}



////////////////////////
// Client Server      //
////////////////////////



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

rpc_command_t receive_command()
{
    rpc_command_t rpc_command;
    return rpc_command;

}

void server(void)
{// server should listen on his own port only
    size_t i;
    const size_t my_tid = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextServerPort();
    //port_id_t destPort = -1;
    size_t clientID = 0;
    const size_t MAX_STRING_SIZE = 4096;

    chunk_t string_table[10];
    for(i = 0; i < 10; ++i)
    {
        string_table[i].len = 0;
        string_table[i].start = (char*)malloc(MAX_STRING_SIZE);
    }
    

    printf("Start server[%lu], listening on port %u !\n", my_tid, myPort);

    // slow down the servers so their progress may be observed
    usleep(THREAD_DELAY);


    while(1)
    {
        // server checks for messages on its own port only
        {
            // the server should receive from the client
            message_t response = Receive(&ports[myPort]);

            // slow down the servers so their progress may be observed
            usleep(THREAD_DELAY);

            // get the client's ID and return port

            clientID = response.payload[0].header.source_port;
            //destPort = response.payload[0].header.source_port;

            printf("Server[%lu] recv () on port %u from client[%lu]\n", 
                my_tid, /*response.payload[2], response.payload[3],*/ myPort, clientID);

    //        // set header fields (tid, port) to server's info
    //        response.payload[0] = (message_value_type)my_tid;
    //        response.payload[1] = myPort;

    //        // add the client's data for them
    //        response.payload[4] = response.payload[2] + response.payload[3];
    //        printf("Server[%lu] send result (%d) on port %u to client[%lu]\n", 
    //            my_tid, response.payload[4], destPort, clientID);
    //        
    //        // send back to the client
    //        Send(&ports[destPort], response);

        }
    }
}

void client(void)
{
    //const size_t my_tid = tid(CurrQ(&RunQ));
    //const port_id_t myPort = getNextClientPort();
    //port_id_t destPort = 0; 
    //packet_t packet;
    //
    //int payload[message_size];
    //memset(payload, 0, message_size);
    //int i = randint(NUM_PORTS);
    //int j = randint(NUM_PORTS);
    //// payload contents: {my_tid, myPort, myInteger,...}
    //payload[0] = my_tid;
    //payload[1] = myPort;

    //while(1)
    //{
    //    // slow down the clients so their progress may be observed
    //    usleep(THREAD_DELAY);

    //    payload[2] = i++;
    //    payload[3] = j++;
    //    payload[4] = 0;
    //    destPort = chooseServer(); // randomly choose server port to send to
    //    printf("\tClient[%lu] send (%d,%d) to port %u\n", 
    //        my_tid, payload[2], payload[3], destPort);
    //    // build the message
    //    message_t msg = make_message(payload, message_size);
    //    // the client should send to the server 
    //    Send(&ports[destPort], msg);
    //    // do NOTHING until the server returns the message
    //    // wait on a receive on some port
    //    message_t recvd = Receive(&ports[myPort]);
    //    printf("\tClient[%lu] rcvd: (%d,%d,%d) from server[%d]\n", 
    //        my_tid, recvd.payload[2], recvd.payload[3], recvd.payload[4], recvd.payload[0]);
    //    //sleep(1);
    //}
}

void write_client(void)
{
}

void read_client(void)
{
    const size_t my_tid = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextClientPort();
    //port_id_t destPort = 0; 
    int payload[message_size];
    memset(payload, 0, message_size);
    //int i = randint(NUM_PORTS);
    //int j = randint(NUM_PORTS);
    // payload contents: {my_tid, myPort, myInteger,...}
    payload[0] = my_tid;
    payload[1] = myPort;

    while(1)
    {
        // slow down the clients so their progress may be observed
        usleep(THREAD_DELAY);
    }
    
}

// Built in Self Test 

int test_chunker()
{
    const char msg1[]=
"This is a long m" 
"essage that talk"
"s about all the "
"stuff and doesn'"
"t talk about not"
"hing. That's rig"
"ht its the messa"
"ge.";

    size_t num_chunks = 0;
    chunk_t* c = make_chunks(msg1, sizeof(msg1), &num_chunks);
    assert(num_chunks == 8);

    assert(c[0].start[0] == 'T');
    assert(c[0].len == PAYLOAD_SIZE);
    
    assert(c[1].start[0] == 'e');
    assert(c[1].len == PAYLOAD_SIZE);
   
    assert(c[2].start[0] == 's');
    assert(c[2].len == PAYLOAD_SIZE);
   
    assert(c[3].start[0] == 's');
    assert(c[3].len == PAYLOAD_SIZE);
   
    assert(c[4].start[0] == 't');
    assert(c[4].len == PAYLOAD_SIZE);
   
    assert(c[5].start[0] == 'h');
    assert(c[5].len == PAYLOAD_SIZE);
   
    assert(c[6].start[0] == 'h');
    assert(c[6].len == PAYLOAD_SIZE);
   
    assert(c[7].start[0] == 'g');
    assert(c[7].len == 4); //3 + the null
    free(c);

    return EXIT_SUCCESS;

}

int test_packets01()
{
    const char msg[]=
"Brian Fantana: I"
" think I was in "
"love once.      "
"Ron Burgundy: Re"
"ally? What was h"
"er name?        "
"Brian Fantana: I"
"don't remember. "
"Ron Burgundy: Th"
"at's not a good "
"start, but keep "
"going.          "
"Brian Fantana: S"
"he was Brazilian"
", or Chinese, or"
" something weird"
". I met her in t"
"he bathroom of a"
" K-Mart and we m"
"ade love for hou"
"rs. Then we part"
"ed ways, never t"
"o see each other"
" again.         "
"Ron Burgundy: I'"
"m pretty sure th"
"at's not love.  "
"Brian Fantana: D"
"amn it!";
    size_t num_packets = 0;
    packet_t* packets = make_packet(
        msg, 
        sizeof(msg), 
        1,
        6,
        &num_packets);
    assert(num_packets == 29);
    assert(packets[28].header.sequence_number == 28);
    assert(packets[0].header.sequence_number == 0);
    assert(packets[28].header.total_num_packets == 29);
    assert(packets[0].header.total_num_packets== 29);
    assert(packets[28].payload.len == 8);
    assert(packets[0].payload.len == PAYLOAD_SIZE);

    return EXIT_SUCCESS;
}


int do_tests()
{
    assert(test_chunker() == 0 && "Chunker Failed.");
    assert(test_packets01() == 0 && "Packets failed");

    printf("All tests passed.\n");
    return EXIT_SUCCESS;
}

//End Build in tests


int main(int argc, const char *argv[])
{
    int i;
    initrand();
    
    assert(do_tests() == 0 && "Tests Failed.");

    int nServer = NUM_SERVERS;
    int nClient = NUM_CLIENTS;
    printf("\nBegin client-server message passing test program\n");
    printf("Implemented as Strategy 2: 1 mutex, 1 producer sem and 1 consumer sem per port\n");
    printf("Servers perform addition on two integers sent by clients and pass back the result\n");
    printf("Spawning %d servers listening on ports 0 to %d\n", nServer, nServer-1);
    printf("Spawning %d clients with receive ports %d to %d\n\n", nClient, nServer, nServer+nClient-1);
    printf("Press 'Enter' to continue:\n");
    getchar();

    InitQ(&RunQ);
    // Declare a set (array of ports). The ports are numbered 0 to 99.
    for(i = 0; i < NUM_PORTS; ++i)
        PortInit(&ports[i], PORT_DEPTH);
    
    // start multiple clients
    for(i = NUM_SERVERS; i < (NUM_SERVERS+NUM_CLIENTS) ; ++i)
        start_thread(client);

    // start 10 servers, listening on ports 0 to 9 (the "known" ports)
    for(i = 0; i < NUM_SERVERS; ++i)
        start_thread(server);
    

    run();
    return 0;
}


