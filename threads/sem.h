#pragma once
#include "threads.h"

typedef struct _semaphore_t
{
    int count;
    Q WaitQ;
} semaphore_t;


void init_sem(semaphore_t* s, int val)
{
    InitQ(&s->WaitQ);
}

void sem_yield(semaphore_t* sem)
{
    AddQ(&sem->WaitQ, DelQ(&RunQ));
    swapcontext(&sem->WaitQ.curr->ctx, &RunQ.curr->ctx);
}

void P(semaphore_t* sem)
{
    --sem->count;
    if(sem->count < 0)
    {
        sem_yield(sem);
    }
}

void V(semaphore_t* sem)
{
    ++sem->count;
    if(sem->count <= 0)
    {   
        AddQ(&RunQ, DelQ(&sem->WaitQ));
    }
    yield();
}



