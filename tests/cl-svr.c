/*******************************************************************************
 * FILENAME:    cl-svr.c
 * DESCRIPTION: Test program for message passing system for RPC behavior
 *
 * This implementation uses "header file polymorphism" which uses typedefs to
 * switch the underlying type of the queues, and messages. In lieu of C++
 * style templates this allows us to parameterize the functions without
 * changing code. In C this is a good pattern for code reuse, while allowing us to
 * satisfy the requirement of not changing the message implementation.
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

#ifdef DEBUG
    #define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements
#endif
#define VERBOSE_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
    #define dbprint printf
#else
    #define dbprint(format, args...) ((void)0)
#endif
#if VERBOSE_PRINT_ENABLED
    #define vbprint printf
#else
    #define vbprint(format, args...) ((void)0)
#endif

typedef unsigned int port_id_t;

typedef enum _command_t { Add, Delete, Read, AddACK, DeleteACK, ReadACK} command_t;

typedef struct _header_t {
    port_id_t source_port;
    port_id_t dest_port;
    size_t sequence_number;
    size_t total_num_packets;
    command_t command;
    size_t idx; // table row index
    bool morePkts;
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
#define PORT_DEPTH          10  // make N=TABLE_ENTRIES*PKTS_PER_ROW*3=10*4*3
// Declare a set (array of ports). The ports are numbered 0 to NUM_PORTS-1.
#define NUM_PORTS           10 // only 3 clients and 1 server here, 100 is overkill
#define NUM_SERVERS 1
#define NUM_CLIENTS 2  // should not exceed NUM_PORTS - NUM_SERVERS
#define TABLE_ENTRIES 10 // number of rows in the string table
const size_t MAX_STRING_SIZE = 4096;
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

packet_t* makeEOFPkt(
        size_t source_port,
        size_t dest_port)
{
    // this function makes an empty packet that onl;y has the morePkts (EOF) bit unset
    size_t number_of_packets = 1;
    packet_t* packets = (packet_t*)malloc(sizeof(packet_t) * number_of_packets);
    packets[0].header.sequence_number = 0;
    packets[0].header.total_num_packets = number_of_packets;
    packets[0].header.source_port = source_port;
    packets[0].header.dest_port = dest_port;
    packets[0].header.morePkts = false; // false, no more packets
    packets[0].header.idx=100; // secondary signal
    packets[0].payload.len = 0; // no payload
    return packets;
}

packet_t* make_packet(
        char* msg, 
        size_t len, 
        size_t source_port,
        size_t dest_port,
        size_t* /* [out] */ number_of_packets)
{
    size_t i = 0;
    chunk_t* c = make_chunks(msg, len, number_of_packets);
    packet_t* packets = (packet_t*)malloc(sizeof(packet_t) * *number_of_packets);
    
    for(i = 0; i < *number_of_packets; ++i)
    {
        packets[i].header.sequence_number = i;
        packets[i].header.total_num_packets = *number_of_packets;
        packets[i].header.source_port = source_port;
        packets[i].header.dest_port = dest_port;
        packets[i].header.morePkts = true;
        packets[i].payload.len = c[i].len;
        memcpy(packets[i].payload.payload, c[i].start, c[i].len);
    }
    free(c);
    return packets;
}

void trace_client(int client, void* t){  P(t); }
void debug_trace(int client, void* t){ V(t); }


////////////////////////
// Client Server      //
////////////////////////

const char st_crispin_day[]="\
That he which hath no stomach to this fight,\
Let him depart; his passport shall be made,\
And crowns for convoy put into his purse;\
We would not die in that man's company\
That fears his fellowship to die with us.\
This day is call'd the feast of Crispian.\
He that outlives this day, and comes safe home,\
Will stand a tip-toe when this day is nam’d,\
And rouse him at the name of Crispian.\
He that shall live this day, and see old age,\
Will yearly on the vigil feast his neighbours,\
And say 'To-morrow is Saint Crispian.'\
Then will he strip his sleeve and show his scars,\
And say 'These wounds I had on Crispian’s day.'\
Old men forget; yet all shall be forgot,\
But he’ll remember, with advantages,\
What feats he did that day. Then shall our names,\
Familiar in his mouth as household words-\
Harry the King, Bedford and Exeter,\
Warwick and Talbot, Salisbury and Gloucester-\
Be in their flowing cups freshly rememb’red.\
This story shall the good man teach his son;\
And Crispin Crispian shall ne'er go by,\
From this day to the ending of the world,\
But we in it shall be remembered-\
We few, we happy few, we band of brothers;\
For he to-day that sheds his blood with me\
Shall be my brother; be he ne'er so vile,\
This day shall gentle his condition;\
And gentlemen in England now-a-bed\
Shall think themselves accurs'd they were not here,\
And hold their manhoods cheap whiles any speaks\
That fought with us upon Saint Crispin's day.";\

int randint(int max) {
    int randomNumber = (int)rand();
    int retVal = randomNumber % max;
    return retVal;
} 

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

semaphore_t t;
void initrand() 
{ 
    srand((unsigned)(time(0))); 
    t = CreateSem(1); 
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
        printf("\t[%d] \"%s\"\n", i, table[i].start);
    }
    printf("\n");
}


void server(void)
{// server should listen on his own port only
    size_t i;
    int j;
    const size_t myTID = tid(CurrQ(&RunQ));
    const port_id_t myPort = getNextServerPort();
    chunk_t serverTable[TABLE_ENTRIES];
    message_t pendingRequestBuffer[2];
    
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
        vbprint("Server waiting on command packet\n");
        message_t recvd = Receive(&ports[myPort]);
        header_t head = recvd.payload[0].header;
        command_t clientCommand = head.command;

        switch(clientCommand)
        {
           case Add:
                if(recvd.payload[0].header.sequence_number == 0) {
                    printf("Server received Add request {c:%u, row:%lu}\n", 
                        head.source_port-1, head.idx);
                }
                tableIdx = recvd.payload[0].header.idx % TABLE_ENTRIES; 
                stringLen = recvd.payload[0].header.total_num_packets*PAYLOAD_SIZE;

                serverTable[tableIdx].len = stringLen;
                size_t start_position = recvd.payload[0].header.sequence_number*PAYLOAD_SIZE;
                memcpy(
                        serverTable[tableIdx].start + start_position,
                        recvd.payload[0].payload.payload,
                        recvd.payload[0].payload.len);
#if VERBOSE_PRINT_ENABLED
                if( recvd.payload[0].header.morePkts == false)
                {
                    printf("Server ");
                    print_table(serverTable, TABLE_ENTRIES);
                }
#endif
                break;
            case Delete:
                printf("Server received Delete request {c:%u, row:%lu}\n", 
                    head.source_port-1, head.idx);
                tableIdx = recvd.payload[0].header.idx % TABLE_ENTRIES;
                serverTable[tableIdx].len = 0;
                break;

            case Read:
                {
                    int dest = recvd.payload[0].header.source_port;
                    printf("Server received read request, sourcePort=%d\n", dest);
                    int totalPktsSent=0;
                    for(j = 0; j < TABLE_ENTRIES; ++j)
                    {
                        packet_t* packets = make_packet(
                                serverTable[j].start,
                                serverTable[j].len,
                                myPort,
                                dest,
                                &n);
                        dbprint("Server sending table[%d](%lu)\n", j, n);
                        if(n == 0) // no need to send more than one packet
                        {// account for empty table rows
                            message_t msg;
                            msg.payload_size = 1;
                            msg.payload[0].payload.len = 0;
                            msg.payload[0].header.command = Read;
                            msg.payload[0].header.sequence_number = 0;
                            msg.payload[0].header.idx = j;
                            msg.payload[0].header.morePkts = true;
                            msg.payload[0].header.total_num_packets = 1;
                            Send(&ports[dest], msg);
                            totalPktsSent++;
                        }
                        // create a header for return message
                        for(i = 0; i < n; ++i)
                        {
                            dbprint("\tServer sending table[%d], pkt %lu/%lu \n",
                                j, i+1, n);
                            message_t msg;
                            msg.payload_size = 1;
                            memcpy(msg.payload, &packets[i], sizeof(message_value_type));
                            msg.payload[0].header.idx = j;
                            msg.payload[0].header.command = Read;
                            //fprintf(stderr, "Sending to: %d\n", dest);
                            Send(&ports[dest], msg);
                            totalPktsSent++;
                        }
                        free(packets);
                    }
                    // Server should send an "end of flow" packet here with header.morePkts=0
                    dbprint("Server done sending data, send EOF\n");
                    packet_t *lastPacket = makeEOFPkt(myPort, dest);
                    message_t msg;
                    msg.payload_size = 1;
                    memcpy(msg.payload, &lastPacket[0], sizeof(message_value_type));
                    msg.payload[0].header.idx = 100;
                    msg.payload[0].header.morePkts = false;
                    msg.payload[0].header.command = Read;
                    Send(&ports[dest], msg);
                    free(lastPacket);
                    dbprint("Server completed <%u> request, total=%d\n", 
                            clientCommand, totalPktsSent);
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
    const size_t total_msgs = sizeof(st_crispin_day)/PAYLOAD_SIZE;
    size_t current_msg = 0;
    size_t const WRITE_SIZE = PAYLOAD_SIZE*4;
    char msg[WRITE_SIZE+1];
    int i;
    int tableIdx = 0;
    printf("Starting Client #%d [TID=%lu], with receive port %u !\n", clientID, myTID, myPort);
    memset(msg, 0, sizeof(msg));
    while(1)
    {
        // slow down writers
        usleep(THREAD_DELAY);  

        // randomly choose a string form the library
        current_msg = randint(total_msgs-4);

        size_t n = 0;
        packet_t* packets;
        if(command == 0)
        {
            // set the message to a string from the library
            memcpy(msg, st_crispin_day+(PAYLOAD_SIZE*current_msg), WRITE_SIZE);
            packets = make_packet(msg, WRITE_SIZE, myPort, serverPort, &n);
        }
        else
        {
            // set the message to empty to delete the string
            memset(msg, 0, PAYLOAD_SIZE);
            packets = make_packet("", PAYLOAD_SIZE, myPort, serverPort, &n);
        }

        // tableIdx selection needs to be random, per spec 
        // "Client 1 and client 2, add/delete or modify the strings, at random."
        // random behavior will cause race conditions so need locks on the table rows
        tableIdx = current_msg % TABLE_ENTRIES;
        // lock table access here so only one client may modify the table at a time
        trace_client(clientID, &t);
        printf("Client #%d sending, row %d: <%s>\n",
                clientID, tableIdx,msg);
        for(i = 0; i < n; ++i)
        {
            message_t msg;
            msg.payload_size = 1;
            packets[i].header.command = command;
            packets[i].header.idx = tableIdx;
            if(i+1==n)
                packets[i].header.morePkts = false;
            memcpy(msg.payload, &packets[i], sizeof(message_value_type));
            dbprint("Client #%d sending message %d of %lu...\n",
                clientID, i+1, n);
            Send(&ports[serverPort], msg);
        }
        free(packets);
        debug_trace(clientID, &t);

        // this needs to be random, per spec: 
        // "Client 1 and client 2, add/delete or modify the strings, at random."
        if(randint(9) < 3)  // // only delete 33% of the time
            command = 1; // Delete;
        else
            command = 0; // Add
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
    const int turns = 1000;
    const int yieldFreq = 960; 
    int yieldRoll = 0;
    int yieldCount = 0;
    chunk_t clientTable[TABLE_ENTRIES];

     // initialize string table
    for(i = 0; i < TABLE_ENTRIES; ++i)
    {
        clientTable[i].len = 0;
        clientTable[i].start = (char*)malloc(MAX_STRING_SIZE);
        memset(clientTable[i].start, 0, MAX_STRING_SIZE);
    }
    
    yield();
    printf("Starting Client #%d [TID=%lu], with receive port %u !\n", clientID, myTID, myPort);
    
    while(1)
    {
        yieldRoll = randint(turns); 
        vbprint("Yield roll = %d\n", yieldRoll);
        if(yieldRoll > yieldFreq){// 
            // slow down the clients so their progress may be observed
            usleep(THREAD_DELAY);
            message_t msg;
            packet_t packet;
            msg.payload_size = 1;
            packet.header.command = Read;
            packet.header.source_port = myPort;
            packet.header.dest_port = serverPort;

            memcpy(msg.payload, &packet, sizeof(message_value_type));
            trace_client(clientID, &t); 
            Send(&ports[serverPort], msg);
            printf("Client #%d sent Read request to server...\n", clientID);
            vbprint("Client #%d waiting on receive from server...\n", clientID);
            // Receive the first message for the first row of the table
            // the below algorithm assumes that all of the packets are sent and 
            // received in order.  This is flawed as we know that the queue (network)
            // does not always deliver them in order
            int tableRow = 0;
            int totalPktsRcv=0;
            do{  // this outer do-while represents the table rows
                vbprint("Client #%d waiting on receive:  table[%d]. first packet\n", 
                        clientID, tableRow);
                int pktRcvThisRow = 0;
                int seqNum = 0;
                size_t start_position = 0;
                int packetsThisRow = 0;
                do{// this inner do-while represents the multiple packets of a single row
                    // CAUTION: this assumes that packets of a given row will arrive in order
                    msg = Receive(&ports[myPort]);
                    pktRcvThisRow++;
                    totalPktsRcv++;
                    header_t header = msg.payload[0].header;
                    tableRow = header.idx;
                    packetsThisRow = header.total_num_packets;
                    seqNum = header.sequence_number; 
                    start_position = seqNum * PAYLOAD_SIZE;
                    if(tableRow < TABLE_ENTRIES){
                        clientTable[tableRow].len = header.total_num_packets * PAYLOAD_SIZE;      
                        dbprint("Client #%d received table[%d], pkt %d/%d, tot=%d\n", 
                                clientID, tableRow, pktRcvThisRow, packetsThisRow, totalPktsRcv);
                        memcpy(
                                clientTable[tableRow].start + start_position,
                                msg.payload[0].payload.payload,
                                msg.payload[0].payload.len);
                        vbprint("Client #%d copied data: table[%d], pkt %d/%d\n", 
                                clientID, tableRow, pktRcvThisRow, packetsThisRow);
                    }else{
                        dbprint("Client #%d received end-of-flow packet", clientID);
                    }
                }while(pktRcvThisRow < packetsThisRow);
            }while(msg.payload[0].header.morePkts == true );
            debug_trace(clientID, &t); 
            printf("\nClient #%d ", clientID);
            print_table(clientTable, TABLE_ENTRIES);
        }else{
            // always yield unless the "magic number" has been hit
            dbprint("Client #%d, yieldng (count = %d)\n", clientID, yieldCount);
            yieldCount ++;
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
"Integer dictum elementum orci, in dictum enim condimentum non. Proin"
"tincidunt, magna nec pellentesque elementum, tellus ligula convallis orci, in"
"finibus justo eros a diam. Vestibulum consequat, nibh vitae mollis dapibus,"
"ligula elit euismod leo, eget auctor felis tellus ut nisi. Sed"
"volutpat congue lorem, vel vulputate orci tristique eu. Vestibulum"
"ac magna mi. Nunc auctor aliquet tellus eget semper. Curabitur"
"pretium enim ante, at fermentum orci posuere a."
"Vestibulum posuere placerat mauris et vulputate. Sed id lacinia leo, nec"
"interdum magna. Vestibulum interdum, nulla et feugiat pulvinar, dolor sapien"
"blandit lacus, eu rutrum erat nisi et nulla. Vestibulum gravida pulvinar"
"nulla, sed condimentum sem fermentum vel. Fusce quis faucibus tellus, vel"
"vehicula ante. Mauris eleifend, enim eu lacinia ornare, nibh diam vestibulum"
"nibh, et dapibus enim metus mollis tortor. Quisque eu vehicula tellus. Cras"
"vehicula odio convallis porttitor eleifend. Donec sollicitudin elit eget urna"
"gravida, vel rutrum neque aliquam."
"Etiam in dignissim purus. Etiam lobortis orci ac sem faucibus, non venenatis"
"urna condimentum. Vivamus id urna quis metus vehicula elementum eu id augue."
"Suspendisse a magna tempus, facilisis dolor non, mattis lacus. Vestibulum sem"
"felis, fringilla eget rhoncus quis, dictum at dui. Mauris eget mi nec libero"
"efficitur aliquet vel placerat diam. Mauris pulvinar nunc ut sapien fermentum"
"congue. Cras eu dapibus odio. Nam euismod lorem in leo egestas, non suscipit"
"nulla consequat. Nulla nec laoreet eros. In congue fringilla est, eget"
"molestie dui lobortis ac. Sed ac metus consequat orci dignissim ultricies."
"Curabitur dictum est eros, a interdum ex porttitor ac. Nullam non velit quis"
"tellus suscipit sodales non vitae quam. Mauris ac sagittis nunc. Vestibulum"
"vestibulum risus at blandit tincidunt. Vestibulum dictum lorem vel diam"
"ullamcorper, a mollis odio rhoncus. Sed convallis, velit eu lobortis mattis,"
"ante urna molestie libero, vel posuere lacus justo nec dui. Donec feugiat,"
"ligula quis eleifend pretium, tellus nulla commodo turpis, quis rutrum enim"
"orci et lacus. Nulla sit amet augue leo. Donec semper pretium mi, nec"
"vehicula magna accumsan et. Fusce vulputate volutpat suscipit. Proin nec"
"nisi sed nisl egestas semper.";
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

    printf("\nBegin client-server string storage test program\n");
    printf("Currently implemented multi-packet strings and working on client read\n");
    printf("Server stores a table of strings (%d) and supports the following operations: \n", TABLE_ENTRIES);
    printf("\tAdd(i,msg): add a string to the table at a specific index, overwriting the contents of that row\n");
    printf("\tDelete(i):  remove the contents of the table at a specific index\n");
    printf("\tRead:       read out the entire contents of the table\n");


    printf("\n\tThis implementation uses ");
    printf(ANSI_COLOR_GREEN "header file polymorphism " ANSI_COLOR_RESET); 
    printf("which uses typedefs to\n"
"\tswitch the underlying type of the queues, and messages. In lieu of C++\n"
"\tstyle templates this allows us to parameterize the functions ");
printf(ANSI_COLOR_GREEN "without\n\tchanging code. " ANSI_COLOR_RESET);
printf("In C, this is a good pattern for code reuse, while allowing \n"
"\tus to satisfy the requirement of not changing the message implementation.\n\n");

    printf("Spawning %d servers listening on ports 0 to %d\n", NUM_SERVERS, NUM_SERVERS-1);
    printf("Spawning %d clients with receive ports %d to %d\n\n", NUM_CLIENTS, NUM_SERVERS, NUM_SERVERS+NUM_CLIENTS-1);
    printf("Press 'Enter' to continue:\n");
    getchar();

    InitQ(&RunQ);
    // Declare a set (array of ports). The ports are numbered 0 to 99.
    for(i = 0; i < NUM_PORTS; ++i)
        PortInit(&ports[i], PORT_DEPTH);
    
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


