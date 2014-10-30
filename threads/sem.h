#pragma once
#include "threads.h"

#ifdef DEBUG
#define S_TRACE() do{ printf("%s:%d Sem%lu=%d Thread: %lu\n", __func__, __LINE__, sem->sid, sem->count, sem->WaitQ.curr->tid); } while(0);
#define T_TRACE() do{ printf("%s:%d Sem%lu=%d Thread: %lu\n", __func__, __LINE__, sem->sid, sem->count, RunQ.curr->tid); } while(0);
#else
#define S_TRACE()
#define T_TRACE()
#endif
typedef struct _semaphore_t
{
    int count;
    Q WaitQ;
    size_t sid;
} semaphore_t;

void init_sem(semaphore_t* s, int val)
{
    static size_t gid = 0;

    InitQ(&s->WaitQ);
    s->count = val;
    s->sid = ++gid;
}

void sem_yield(semaphore_t* sem)
{
    T_TRACE();
    AddQ(&sem->WaitQ, DelQ(&RunQ));
    swapcontext(&sem->WaitQ.curr->ctx, &RunQ.curr->ctx);
}

void P(semaphore_t* sem)
{
    if(sem->count > 0)
    {
        --sem->count;
    }
    else
    {
        sem_yield(sem);
    }
}

void V(semaphore_t* sem)
{
    ++sem->count;
    T_TRACE();
    if(sem->count <= 0)
    {   
        S_TRACE();
        AddQ(&RunQ, DelQ(&sem->WaitQ));
    }
    yield();
}



