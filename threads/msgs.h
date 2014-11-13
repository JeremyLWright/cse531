#pragma once
#include "sem.h"
#include <assert.h>

typedef struct _message_t {
    size_t payload_size;
    message_value_type payload[message_size];
} message_t;

typedef struct _Port_t {
    semaphore_t mutex;
    semaphore_t producer_sem;
    semaphore_t consumer_sem;
    size_t max_size;
    size_t write_idx;
    size_t read_idx;
    message_t* data; 
} Port_t;

void PortInit(Port_t* portA, size_t max_size)
{
    portA->max_size = max_size;
    portA->write_idx = 0;
    portA->read_idx = 0;
    portA->data = (message_t*)malloc(sizeof(message_t)*max_size);
    portA->producer_sem = CreateSem(max_size-1);// init to max_size - 1 since that many open spaces
    portA->consumer_sem = CreateSem(0);// should init the consumer to 0 since initially nothing in the buffer
    portA->mutex = CreateSem(1);
}

message_t make_message(message_value_type* payload, size_t payload_size)
{
    message_t msg;
    
    assert(payload_size <= message_size);

    msg.payload_size = payload_size;
    memcpy(msg.payload, payload, payload_size*sizeof(message_value_type));
    return msg;
}

void Send(Port_t* port, const message_t msg)
{
    //P(&port->mutex);
    {
        printf("\tProducer Sem: %d\n", port->producer_sem.count);
        P(&port->producer_sem);
            memcpy(&port->data[port->write_idx], &msg, sizeof(msg));
            port->write_idx++;
            port->write_idx = port->write_idx % port->max_size;
            V(&port->consumer_sem);
    }//  the send should issue a signal here notifying the reader
    // if no signal to the reader, he will never wake up on a successful receive
    //V(&port->mutex);
}

message_t Receive(Port_t* port)
{
    message_t msg;
    //P(&port->mutex);
    {
        P(&port->consumer_sem); //Block if we are the only reader

            memcpy(&msg, &port->data[port->read_idx], sizeof(message_t));
            port->read_idx++;
            port->read_idx = port->read_idx % port->max_size;

        //V(&port->consumer_sem); //Unblock any waiting readers 
        // if the consumer unblocks waiting readers, they wil get double-unblocked by writer
            V(&port->producer_sem); //Unblock any waiting producers to notify we read
            printf("\t\tConsumer Sem: %d\n", port->consumer_sem.count);
            printf("\t\tProducer Sem: %d\n", port->producer_sem.count);
    }
    //V(&port->mutex);
    return msg;
}
