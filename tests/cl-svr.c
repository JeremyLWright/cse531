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
semaphore_t tableMutex;

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

void sem_wait(int client, semaphore_t* sem){
    const char semOp [] = "P()";
    printf("\tClient #%d %s enter, oldCount=%d, newCount=%d\n", 
        client, semOp, sem->count, sem->count-1);
    P(sem);
    printf("\tClient #%d %s exit, count=%d\n", 
        client, semOp, sem->count);
}

void sem_signal(int client, semaphore_t* sem){
    const char semOp [] = "V()";
    printf("\tClient #%d %s enter, oldCount=%d, newCount=%d\n", 
        client, semOp, sem->count, sem->count+1);
    V(sem);
    printf("\tClient #%d %s exit, count=%d\n", 
        client, semOp, sem->count);
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
    // no need for locks here since each table is private to the respective host
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
    chunk_t serverTable[TABLE_ENTRIES];
    
    printf("Starting server[TID=%lu], listening on port %u !\n", myTID, myPort);

    // initialize string table
    for(i = 0; i < TABLE_ENTRIES; ++i)
    {
        serverTable[i].len = 0;
        serverTable[i].start = (char*)malloc(MAX_STRING_SIZE);
        memset(serverTable[i].start, 0, MAX_STRING_SIZE);
    }
    

    size_t n = 0;
    int tableIdx = 0;
    size_t stringLen = 0;


    while(1)
    {
        
        // slow down the servers so their progress may be observed
        usleep(THREAD_DELAY);
        message_t recvd = Receive(&ports[myPort]);
        header_t head = recvd.payload[0].header;
        switch(recvd.payload[0].header.command)
        {
           case Add:
                printf("Server received Add request {c:%u, row:%lu}\n", 
                    head.source_port-1, head.idx);
                tableIdx = recvd.payload[0].header.idx % TABLE_ENTRIES;  // TODO should thie be TABLE_ENTRIES OR PORT_DEPTH??
                stringLen = recvd.payload[0].header.total_num_packets*PAYLOAD_SIZE;

                serverTable[tableIdx].len = stringLen;
                size_t start_position = recvd.payload[0].header.sequence_number*PAYLOAD_SIZE;
                memcpy(
                        serverTable[tableIdx].start + start_position,
                        recvd.payload[0].payload.payload,
                        recvd.payload[0].payload.len);
                ////TODO This needs to be moved to the read client
                if(counter > 100000)
                {
                    printf("Server printing his received table\n");
                    counter = 0;
                    print_table(serverTable, TABLE_ENTRIES);
                }
                counter++;
                ////End TODO
                break;
            case Delete:
                printf("Server received Delete request {c:%u, row:%lu}\n", 
                    head.source_port-1, head.idx);
                tableIdx = recvd.payload[0].header.idx % TABLE_ENTRIES;
                serverTable[tableIdx].len = 0;
                break;

            case Read:
                {
                    printf("Server received read request\n");
                    int dest = recvd.payload[0].header.source_port;
                    for(j = 0; j < TABLE_ENTRIES; ++j)
                    {
                        printf("Server sending table[%d]\n", j);
                        packet_t* packets = make_packet(
                                serverTable[j].start,
                                serverTable[j].len,
                                myPort,
                                dest,
                                &n);
                        if(n == 0)
                        {
                            message_t msg;
                            msg.payload_size = 1;
                            msg.payload[0].payload.len = 0;
                            Send(&ports[dest], msg);
                        }

                        for(i = 0; i < n; ++i)
                        {
                            printf("\tServer sending table[%d], pkt %lu/%lu \n",
                                j, i+1, n);
                            message_t msg;
                            msg.payload_size = 1;
                            memcpy(msg.payload, &packets[i], sizeof(message_value_type));
                            msg.payload[0].header.idx = j;
                            //fprintf(stderr, "Sending to: %d\n", dest);
                            Send(&ports[dest], msg);
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

        size_t n = 0;
        packet_t* packets;
        if(command == 0)// TODO: these cases should be two different client functions
        {
        // set the message to a string from the library
            memcpy(msg, anchor_man_01+(PAYLOAD_SIZE*current_msg), WRITE_SIZE);
            packets = make_packet(msg, WRITE_SIZE, myPort, serverPort, &n);
        }
        else
        {
        // set the message to empty to delete the string
            // TODO: client should only send a single message if the command is delete
            memset(msg, 0, PAYLOAD_SIZE);
            packets = make_packet("", PAYLOAD_SIZE, myPort, serverPort, &n);
        }

        // TODO tableIdx selection needs to be random, per spec?? 
        // "Client 1 and client 2, add/delete or modify the strings, at random."
        // random behavior will cause race conditions so need locks on the table rows
        tableIdx = current_msg % TABLE_ENTRIES;
        // lock able access here so only one client may modify the table at a time
        sem_wait(clientID, &tableMutex); 
        printf("Client #%d sending to server, row %d, in %lu messages string <%s> ...\n",
                clientID, tableIdx, n, msg);
        for(i = 0; i < n; ++i)
        {
            message_t msg;
            msg.payload_size = 1;
            packets[i].header.command = command;
            packets[i].header.idx = tableIdx;
            memcpy(msg.payload, &packets[i], sizeof(message_value_type));
            printf("client #%d sending message %d of %lu...\n",
                clientID, i+1, n);
            Send(&ports[serverPort], msg);
        }
        free(packets);
        // unlock the table so that the otner client may access it
        sem_signal(clientID, &tableMutex);

        // cycle through commands of adding a string or deleting a string 
        // TODO = this needs to be random, per spec: 
        // "Client 1 and client 2, add/delete or modify the strings, at random."
        command = (command + 1) % 2;

    }
}


void read_client(void)
{
    const size_t myTID = tid(CurrQ(&RunQ));
    const int clientID = 3;
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
    int yieldCount = 1;
    chunk_t clientTable[TABLE_ENTRIES];

    yield();
    printf("Starting client #%d [TID=%lu], with receive port %u !\n", clientID, myTID, myPort);
    
    while(1)
    {
        yieldCount++;
        if(yieldCount % 19 == 0){
            // only print if the yield count is at a magic number
            // slow down the clients so their progress may be observed
            usleep(THREAD_DELAY);
            message_t msg;
            packet_t packet;
            msg.payload_size = 1;
            packet.header.command = Read;
            packet.header.source_port = myPort;
            packet.header.dest_port = serverPort;

            memcpy(msg.payload, &packet, sizeof(message_value_type));
            // TODO: lock table so that no other clients may write while this one is reading
            //printf("Client #%d locking table\n", clientID);
            //sem_wait(clientID, &tableMutex); 
            //////// Matt... The dead lock is happing with an interaction between
            //the server sending lots of packets and us trying to receieve them.  
            //printf("Client #%d sending Read request to server...\n", clientID);
            //Send(&ports[serverPort], msg);
            printf("Client #%d waiting on receive from server...\n", clientID);
            msg = Receive(&ports[myPort]);
            for(i = 1; i < msg.payload[0].header.total_num_packets; ++i)
            {
                msg = Receive(&ports[myPort]);
                tableIdx = msg.payload[0].header.idx%10; // TODO: why modulo by 10?  is this TABLE_ENTRIES?
                stringLen = msg.payload[0].header.total_num_packets*PAYLOAD_SIZE;
                clientTable[tableIdx].len = stringLen;
                size_t start_position = msg.payload[0].header.sequence_number*PAYLOAD_SIZE;
                memcpy(
                        clientTable[msg.payload[0].header.idx].start + start_position,
                        msg.payload[0].payload.payload,
                        msg.payload[0].payload.len);
            }
            // TODO: unlock entire table so that other clients may write
            //printf("Client #%d locking table\n", clientID);
            //sem_signal(clientID, &tableMutex); 
            printf("Client #%d printing his received table\n", clientID);
            print_table(clientTable, TABLE_ENTRIES);
        }else{
            // always yield unless the "magic number" has been hit
            // printf("Client #%d, yieldng (count = %d)\n", clientID, yieldCount);
            yield();
        }
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

    printf("\nBegin client-server string storage test program\n");
    printf("Currently implemented multi-packet strings and working on client read\n");
    printf("Server stores a table of strings (%d) and supports the followign operations: \n", TABLE_ENTRIES);
    printf("\tAdd(i,msg): add a string to the table at a specific index, overwriting the contents of that row\n");
    printf("\tDelete(i):  remove the contents of the table at a specific index\n");
    printf("\tRead:       read out the entire contents of the table\n");
    printf("\tRead(i):    TODO: read out the contents of a specified row of the table\n\n");
    printf("Spawning %d servers listening on ports 0 to %d\n", NUM_SERVERS, NUM_SERVERS-1);
    printf("Spawning %d clients with receive ports %d to %d\n\n", NUM_CLIENTS, NUM_SERVERS, NUM_SERVERS+NUM_CLIENTS-1);
    printf("Press 'Enter' to continue:\n");
    getchar();

    InitQ(&RunQ);
    // Declare a set (array of ports). The ports are numbered 0 to 99.
    for(i = 0; i < NUM_PORTS; ++i)
        PortInit(&ports[i], PORT_DEPTH);
    
    // declare a table mutex to prevent race conditions for the clients
    tableMutex = CreateSem(1);   


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


