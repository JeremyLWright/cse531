/*******************************************************************************
 * FILENAME:    sem.h
 * DESCRIPTION: Semaphore implementation using user level threads
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Distributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#pragma once
#include "threads.h"

#ifdef DEBUG
#define S_TRACE() do{ printf(ANSI_COLOR_GREEN "%s:%d Sem%lu=%d Thread: %lu\n" ANSI_COLOR_RESET,  __func__, __LINE__, sem->sid, sem->count, tid(sem->WaitQ.curr)); } while(0);
#define T_TRACE() do{ printf(ANSI_COLOR_BLUE"%s:%d Sem%lu=%d Thread: %lu\n" ANSI_COLOR_RESET, __func__, __LINE__, sem->sid, sem->count, tid(RunQ.curr)); } while(0);
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

semaphore_t CreateSem(int val)
{
    static size_t gid = 0;
    semaphore_t s;
    InitQ(&s.WaitQ);
    s.count = val;
    s.wakeups = 0;
    s.sid = ++gid;
    return s;
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

void P(semaphore_t* sem)
{
    sem->count--;
    if(sem->count < 0)
    {
        //Save the current thread.
        list_value_type* current_thread = CurrQ(&RunQ);
        AddQ(&sem->WaitQ, DelQ(&RunQ)); //Adding does not set the current pointer, so we need a temporary
#ifdef DEBUG
        T_TRACE();
        {//Print the queues for prettiness
            char* c = (char*)malloc(4096);
            Print(c, &RunQ);
            printf(ANSI_COLOR_RED "RunQ: %s" ANSI_COLOR_RESET, c);
            Print(c, &sem->WaitQ);
            printf(ANSI_COLOR_RED "SemQ: %s" ANSI_COLOR_RESET, c);
            free(c);
        }
        printf(ANSI_COLOR_YELLOW "T%lu blocking sem_yield().\n" ANSI_COLOR_RESET, tid(current_thread));
#endif
        swapcontext(&current_thread->ctx, &CurrQ(&RunQ)->ctx);
#ifdef DEBUG
        printf(ANSI_COLOR_YELLOW "T%lu return from sem_yield().\n" ANSI_COLOR_RESET, tid(current_thread));
#endif
    }
}

void V(semaphore_t* sem)
{
    T_TRACE();
    sem->count++;
    if(sem->count <= 0)
    {   
#ifdef DEBUG
        printf(ANSI_COLOR_MAGENTA "Unblocking thread.\n" ANSI_COLOR_RESET);
        S_TRACE();
#endif
        AddQ(&RunQ, DelQ(&sem->WaitQ));
    }
    yield();
}



