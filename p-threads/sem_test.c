// (c) Partha Dasgupta 2009
// permission to use and distribute granted.

#include <stdio.h>
#include <stdlib.h>
#include "threads.h"

typedef struct semaphore_t {
     pthread_mutex_t mutex;
     pthread_cond_t cond;
     int count;
} semaphore_t;

void init_sem(semaphore_t *s, int i)
{   
    s->count = i;
    pthread_mutex_init(&(s->mutex), NULL);
    pthread_cond_init(&(s->cond), NULL);
}


/*
 * The P routine decrements the semaphore, and if the value is less than
 * zero then blocks the process 
 */
void P(semaphore_t *sem)
{   
    pthread_mutex_lock (&(sem->mutex)); 
    sem->count--;
    if (sem->count < 0) pthread_cond_wait(&(sem->cond), &(sem->mutex));
    pthread_mutex_unlock (&(sem->mutex)); 
}


/*
 * The V routine increments the semaphore, and if the value is 0 or
 * negative, wakes up a process and yields
 */

void V(semaphore_t * sem)
{   
    pthread_mutex_lock (&(sem->mutex)); 
    sem->count++;
    if (sem->count <= 0) {
	pthread_cond_signal(&(sem->cond));
    }
    pthread_mutex_unlock (&(sem->mutex)); 
    pthread_yield();
}


semaphore_t mutex;

void function_1(void)
{
    while (1){ 
        P(&mutex);
        printf("Beginning of CS: func 1\n");
        sleep(1);
        printf("End of CCS: func 1..\n");
        sleep(1);
	V(&mutex);
    }
}    

void function_2(void)
{
    while (1){ 
        P(&mutex);
        printf("Beginning of CS: func 2\n");
        sleep(1);
        printf("End of CCS: func 2..\n");
        sleep(1);
	V(&mutex);
    }
}    

void function_3(void)
{
    while (1){ 
        P(&mutex);
        printf("Beginning of CS: func 3\n");
        sleep(1);
        printf("End of CCS: func 3..\n");
        sleep(1);
	V(&mutex);
    }
}    

int main()
{

    init_sem(&mutex, 1);
    start_thread(function_1, NULL);
    start_thread(function_2, NULL);
    start_thread(function_3, NULL);

    while(1) sleep(1);

    return 0;
}




