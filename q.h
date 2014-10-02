#pragma once
#include <stdbool.h>
#include "TCB.h"

typedef struct _Q {
} Q;
void InitQ (Q* q)  //Note that if Q is a head pointer to the queue, then InitQ will have to be passed &Q.
{
}

void AddQ(Q* q, int *item)
{
}

void DelQ(Q* q) // will return a pointer to the item deleted.
{
}

void RotateQ(Q* q) // deletes the head and adds it to the tail, by just moving the header pointer to the next item.
{
}

bool t()
{
    return true;
}
