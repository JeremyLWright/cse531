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
#include <stdbool.h>


///////////////////////////////////////
//  Message Transport (Packets)      // 
///////////////////////////////////////

typedef unsigned int port_id_t;

typedef enum _command_t { Add, Delete, Read} command_t;

typedef struct _header_t {
    port_id_t source_port;
    port_id_t dest_port;
    size_t sequence_number;
    size_t total_num_packets;
    command_t command;
    size_t idx;
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
#define NUM_CLIENTS 2  // should not exceed NUM_PORTS - NUM_SERVERS
#define TABLE_ENTRIES 10 // number of rows in the string table

#define THREAD_DELAY 100000
#include "msgs.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h> // getchar()
#include <stdlib.h> // rand(), srand()
#include <time.h> // time()nistd.h>

Port_t ports[NUM_PORTS];
port_id_t g_next_server_port = 0;
port_id_t g_next_client_port = NUM_SERVERS;
int g_next_client_ID=1;

typedef struct _chunk_t {
    size_t len;
    char* start;
} chunk_t;

chunk_t* make_chunks(
        char* msg,
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
        char* msg, 
        size_t len, 
        size_t source_port,
        size_t dest_port,
        size_t* /* [out] */ number_of_packets)
{// TODO: number_of_packets just needs to be an integer, but we're using a pointer to an integer?
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


////////////////////////
// Client Server      //
////////////////////////

const char anchor_man_01[]=
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
"amn it!         ";

int randint(int max) {
    int randomNumber = (int)rand();
    int retVal = randomNumber % max;
//    printf("randomNumber = %d, retVal = %d\n", randomNumber, retVal);
    return retVal;
} 

void initrand() { srand((unsigned)(time(0))); }

int getNextServerPort(){
    port_id_t myPort = g_next_server_port;
    g_next_server_port++;
    return (myPort);
}

int getNextClientPort(){
    port_id_t myPort = g_next_client_port;
    g_next_client_port++;
    return (myPort);
}
int getClientID(){
    int myID = g_next_client_ID;
    g_next_client_ID++;
    return (myID);
}


port_id_t chooseServer(){
    port_id_t server_port = randint(NUM_SERVERS); 
    return (server_port);
}

void print_table(chunk_t* table, size_t len)
{
    // TODO: need a whole-table lock here to prevent any access while receiving from server
    // might be moot since we have atomicity anyway??
    int i = 0;
    printf("Printing String Table:\n");
    for(i = 0; i < len; ++i)
    {
        printf("\t[%d] Length: %lu - %s\n", i, table[i].len, table[i].start);
    }
}


void server(void)
{// server should listen on his own port only
    size_t i;
    int j;
    const size_t myTID = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextServerPort();
    int counter = 0;
    const size_t MAX_STRING_SIZE = 4096;
    chunk_t string_table[TABLE_ENTRIES];
    
    printf("Starting server[TID=%lu], listening on port %u !\n", myTID, myPort);

    // initialize string table
    for(i = 0; i < TABLE_ENTRIES; ++i)
    {
        string_table[i].len = 0;
        string_table[i].start = (char*)malloc(MAX_STRING_SIZE);
        memset(string_table[i].start, 0, MAX_STRING_SIZE);
    }
    

    size_t n = 0;
    int tableIdx = 0;
    size_t stringLen = 0;


    while(1)
    {
        
        // slow down the servers so their progress may be observed
        usleep(THREAD_DELAY);
        message_t recvd = Receive(&ports[myPort]);
        switch(recvd.payload[0].header.command)
        {
           case Add:
                tableIdx = recvd.payload[0].header.idx % TABLE_ENTRIES;  // TODO should thie be TABLE_ENTRIES OR PORT_DEPTH??
                stringLen = recvd.payload[0].header.total_num_packets*PAYLOAD_SIZE;

                string_table[tableIdx].len = stringLen;
                size_t start_position = recvd.payload[0].header.sequence_number*PAYLOAD_SIZE;
                memcpy(
                        string_table[tableIdx].start + start_position,
                        recvd.payload[0].payload.payload,
                        recvd.payload[0].payload.len);
                ////TODO This needs to be moved to the read client
                if(counter > 100000)
                {
                    counter = 0;
                    print_table(string_table, TABLE_ENTRIES);
                }
                counter++;
                ////End TODO
                break;
            case Delete:
                tableIdx = recvd.payload[0].header.idx % TABLE_ENTRIES;
                string_table[tableIdx].len = 0;
                break;

            case Read:
                {
                    int dest = recvd.payload[0].header.source_port;
                    for(j = 0; j < TABLE_ENTRIES; ++j)
                    {
                        packet_t* packets = make_packet(
                                string_table[j].start,
                                string_table[j].len,
                                myPort,
                                dest,
                                &n);
                        if(n == 0)
                        {
                            message_t m;
                            m.payload_size = 1;
                            m.payload[0].payload.len = 0;
                            Send(&ports[dest], m);
                        }

                        for(i = 0; i < n; ++i)
                        {
                            message_t m;
                            m.payload_size = 1;
                            memcpy(m.payload, &packets[i], sizeof(message_value_type));
                            m.payload[0].header.idx = j;
                            //fprintf(stderr, "Sending to: %d\n", dest);
                            Send(&ports[dest], m);
                        }
                        free(packets);
                    }
                }
                break;
        }
    }
}


void write_client(void)
{
    const size_t myTID = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextClientPort();
    const port_id_t serverPort = chooseServer();
    const int clientID = getClientID();
    uint8_t command = 0;
    const size_t total_msgs = sizeof(anchor_man_01)/PAYLOAD_SIZE;
    size_t current_msg = 0;
    size_t const WRITE_SIZE = PAYLOAD_SIZE*4;
    char msg[WRITE_SIZE];
    int i;
    int tableIdx = 0;
    //printf("Start client[%lu], with receive port %u !\n", myTID, myPort);
    printf("Starting client #%d [TID=%lu], with receive port %u !\n", clientID, myTID, myPort);
    while(1)
    {
        // slow down writers
        usleep(THREAD_DELAY);  
        // TODO: why does a sleep here cause deadlock??

        // randomly choose a string form the library
        current_msg = randint(total_msgs-4);

        if(command == 0)
        {
        // set the message to a string from the library
            memcpy(msg, anchor_man_01+(PAYLOAD_SIZE*current_msg), WRITE_SIZE);
        }
        else
        {
        // set the message to empty to delete the string
            memset(msg, 0, PAYLOAD_SIZE);
        }
        size_t n = 0;
        packet_t* packets = make_packet(msg, WRITE_SIZE, myPort, serverPort, &n);
        // TODO = this needs to be random, per spec: 
        // "Client 1 and client 2, add/delete or modify the strings, at random."
        // random behavior will cause race conditions so need locks on the table rows

        tableIdx = current_msg % TABLE_ENTRIES;
        printf("Client #%d sending to server, row %d, in %lu messages string <%s> ...\n",
                clientID, tableIdx, n, msg);
        // TODO lock table access here so only one client may modify the table at a time
        // P(row_sem)
        for(i = 0; i < n; ++i)
        {
            message_t m;
            m.payload_size = 1;
            packets[i].header.command = command;
            packets[i].header.idx = tableIdx;
            memcpy(m.payload, &packets[i], sizeof(message_value_type));
            printf("client #%d sending message %d of %lu...\n",
                clientID, i+1, n);
            Send(&ports[serverPort], m);
        }
        free(packets);
        // TODO: unlock the row so that the otner client may access it
        // V(row_sem)

        // cycle through commands of adding a string or deleting a string 
        // TODO = this needs to be random, per spec: 
        // "Client 1 and client 2, add/delete or modify the strings, at random."
        command = (command + 1) % 2;

    }
}


void read_client(void)
{
    const size_t myTID = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextClientPort();
    const port_id_t serverPort = chooseServer();
    //port_id_t destPort = 0; 
    int payload[message_size];
    memset(payload, 0, message_size);
    //int i = randint(NUM_PORTS);
    //int j = randint(NUM_PORTS);
    // payload contents: {myTID, myPort, myInteger,...}
    payload[0] = myTID;
    int i = 0;
    int tableIdx = 0;
    int stringLen = 0;
    chunk_t string_table[TABLE_ENTRIES];

    while(1)
    {
        // slow down the clients so their progress may be observed
        usleep(THREAD_DELAY);
        message_t m;
        packet_t packet;
        m.payload_size = 1;
        packet.header.command = Read;
        packet.header.source_port = myPort;
        packet.header.dest_port = serverPort;

        memcpy(m.payload, &packet, sizeof(message_value_type));
        //////// Matt... The dead lock is happing with an interaction between
        //the server sending lots of packets and us trying to receieve them.  
        //Send(&ports[serverPort], m);
        // TODO: lock entire table (each row mutex) so that no other clients may write while this one is reading
        // P(row_sem) * TABLE_ENTRIES
        m = Receive(&ports[myPort]);
        for(i = 1; i < m.payload[0].header.total_num_packets; ++i)
        {
            m = Receive(&ports[myPort]);

            // the read_client should not ever touch the string table directly - it's private to the server
            tableIdx = m.payload[0].header.idx%10; // TODO: why modulo by 10?  is this TABLE_ENTRIES?
            stringLen = m.payload[0].header.total_num_packets*PAYLOAD_SIZE;
            string_table[tableIdx].len = stringLen;
            size_t start_position = m.payload[0].header.sequence_number*PAYLOAD_SIZE;
            memcpy(
                    string_table[m.payload[0].header.idx].start + start_position,
                    m.payload[0].payload.payload,
                    m.payload[0].payload.len);
        }
        // TODO: unlock entire table (each row mutex) so that other clients may write
        // P(row_sem) * TABLE_ENTRIES
        // TODO: this client should not have any access to the string table (it's private to the sender)
        // so he needs his own table
        print_table(string_table, TABLE_ENTRIES);
    }

}

// Built in Self Test 

int test_chunker()
{
    char msg1[]=
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
    
    // TODO: declare an array of row-semaphores to prevent race conditions for the clients
    for (i = 0; i < TABLE_ENTRIES; i++) {
        // initialize row semaphores (lock server)

    }

    start_thread(read_client);
    // start multiple clients
    for(i = NUM_SERVERS; i < (NUM_SERVERS+NUM_CLIENTS) ; ++i)
        start_thread(write_client);

    // start NUM_SERVERS servers, listening on ports 0 to 9 (the "known" ports)
    for(i = 0; i < NUM_SERVERS; ++i)
        start_thread(server);


    run();
    return 0;
}


