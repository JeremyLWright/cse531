#pragma once
#include <stdbool.h>
#include "TCB.h"

typedef struct _Q {
    int data;
    _Q* next;
    _Q* prev;
} Q;

typedef struct _item {
    int t;
} Item;

void InitQ (Q* q)  //Note that if Q is a head pointer to the queue, then InitQ will have to be passed &Q.
{
}

void AddQ(Q* q, Item* item)
{
}

Item* DelQ(Q* q) // will return a pointer to the item deleted.
{
    return 0;
}

void RotateQ(Q* q) // deletes the head and adds it to the tail, by just moving the header pointer to the next item.
{
}

size_t _size(Q* q)
{
    return 1;
}
