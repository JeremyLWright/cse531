#pragma once
#include "sem.h" // include sem.h in the file msgs.h
#include <assert.h>

typedef struct _message_t {
    // Declare a message type. For simplicity, a message is an array of 10 integers.
    size_t payload_size;
    message_value_type payload[message_size];
} message_t;

/* Port is currently implemented as:  
* Strategy 1: A mutex semaphore for all ports, and a producer semaphore and a consumer semaphore - per port
*   Strategy 2: Same as 1, but a mutex per port.
* i.e. one mutex, one producer sem, one consumer sem per port
*/
typedef struct _Port_t { // Declare a port. 
    // There would be semaphores associated with the port, for controlling synchronization.
    semaphore_t mutex;
    semaphore_t producer_sem;
    semaphore_t consumer_sem;
    size_t max_size; // A port is something that contains N messages
    size_t write_idx;
    size_t read_idx;
    // The port “contains” messages hence a port may be a pointer to a Q of messages, or an array of messages.
    message_t* data; 
} Port_t;

void PortInit(Port_t* portA, size_t max_size)
{
    portA->max_size = max_size;
    portA->write_idx = 0;
    portA->read_idx = 0;
    // The port “contains” messages hence a port may be a pointer to a Q of messages, or an array of messages.
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

/* Send (with appropriate arguments).  
 * The send is a “blocking” asynchronous send that is: 
 *    it places the message in the port and returns but blocks if port is full. */
void Send(Port_t* port, const message_t msg)
{
    // one mutex per port so only one process can access the port
    // TODO: sem wait on the port mutex
    //P(&port->mutex);
    {
        printf("\tProducer Sem: %d\n", port->producer_sem.count);
        // sem wait on the port's producer semaphore.  Block if port is full
        P(&port->producer_sem);
            // copy the contents of the message into the ports buffer
            memcpy(&port->data[port->write_idx], &msg, sizeof(msg));
            // increment the write index of the port for the next message
            port->write_idx++;
            // modulo division to rotate to tbe beginning of the buffer if overflow
            port->write_idx = port->write_idx % port->max_size;
            // sem signal to the consumer semaphore to notify there's content in the port
            V(&port->consumer_sem);
            // the send should issue a signal here notifying the reader
            // if no signal to the reader, he will never wake up on a successful receive
    }
    // TODO sem signal on the port mutex when process is finished with the port
    //V(&port->mutex);
}

/* Receive (with appropriate arguments). The receive is a “blocking” receive. */
message_t Receive(Port_t* port)
{
    message_t msg;
    // sem wait on the port mutex
    //P(&port->mutex);
    {// sem wait on the port producer semaphore - check if full
        P(&port->consumer_sem); //Block if we are the only reader
            // copoy port buffer contents into outgoing message
            memcpy(&msg, &port->data[port->read_idx], sizeof(message_t));
            // move read index to the next location
            port->read_idx++;
            // modulo to wrap around
            port->read_idx = port->read_idx % port->max_size;
            // sem signal to waiting producers to indicate there's room in the port buffer
            V(&port->producer_sem); //Unblock any waiting producers to notify we read
            printf("\t\tConsumer Sem: %d\n", port->consumer_sem.count);
            printf("\t\tProducer Sem: %d\n", port->producer_sem.count);
    }
     // sem signal on the lock to indicate we're done with the port
    //V(&port->mutex);
    return msg;
}
