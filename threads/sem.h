#pragma once
#include "threads.h"

#ifdef DEBUG
#define S_TRACE() do{ printf(ANSI_COLOR_GREEN "%s:%d Sem%lu=%d Thread: %lu\n" ANSI_COLOR_RESET,  __func__, __LINE__, sem->sid, sem->count, sem->WaitQ.curr->tid); } while(0);
#define T_TRACE() do{ printf(ANSI_COLOR_BLUE"%s:%d Sem%lu=%d Thread: %lu\n" ANSI_COLOR_RESET, __func__, __LINE__, sem->sid, sem->count, RunQ.curr->tid); } while(0);
#else
#define S_TRACE()
#define T_TRACE()
#endif
typedef struct _semaphore_t
{
    int count;
    int wakeups;
    Q WaitQ;
    size_t sid;
} semaphore_t;

void init_sem(semaphore_t* s, int val)
{
    static size_t gid = 0;

    InitQ(&s->WaitQ);
    s->count = val;
    s->wakeups = 0;
    s->sid = ++gid;
}

#if 0 //INSPIRED_EXAMPLE_FROM_WIKIPEDIA
#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>
 
int main(int argc, const char *argv[]){
	ucontext_t context;
 
	getcontext(&context);
	puts("Hello world");
	sleep(1);
	setcontext(&context);
	return 0;
}
#endif

void sem_yield(semaphore_t* sem)
{
    //swapcontext(&sem->WaitQ.curr->ctx, &RunQ.curr->ctx);
}

void P(semaphore_t* sem)
{
    sem->count--;
    if(sem->count < 0)
    {
        AddQ(&sem->WaitQ, DelQ(&RunQ));
        T_TRACE();
        swapcontext(&sem->WaitQ.curr->ctx, &RunQ.curr->ctx);
        printf(ANSI_COLOR_YELLOW "Return from swap.\n" ANSI_COLOR_RESET);
       // getcontext(&sem->WaitQ.curr->ctx);
       // if(sem->count < 0) setcontext(&RunQ.curr->ctx);
        //sem->wakeups--;
    }
}

void V(semaphore_t* sem)
{
    T_TRACE();
    sem->count++;
    if(sem->count <= 0)
    {   
        printf(ANSI_COLOR_MAGENTA "Unblocking thread.\n" ANSI_COLOR_RESET);
        S_TRACE();
        //sem->wakeups++;
        AddQ(&RunQ, DelQ(&sem->WaitQ));
    }
    yield();
}



