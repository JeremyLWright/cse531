#pragma once
#include "threads.h"

typedef struct _Semaphore_t
{
    int value;
    Q WaitQ;
} Semaphore_t;


Semaphore_t CreateSem(int val)
{
    Semaphore_t s;
    initQ(&s.WaitQ);
    return s;
}

void sem_yield(Semaphore_t* sem)
{
    AddQ(sem->WaitQ, DelQ(RunQ));
    swapcontext(&sem->WaitQ.curr->ctx, &RunQ.curr->next->ctx);
}

void P(Semaphore_t* sem)
{
    --sem->value;
    if(sem->value < 0)
    {
        sem_yield(sem);
    }
}

void V(Semaphore_t* sem)
{
    ++sem->count;
    if(sem->count <= 0)
    {
        AddQ(RunQ, DelQ(sem->WaitQ));
    }
    yield();
}



