// (c) Partha Dasgupta 2009
// permission to use and distribute granted.


#include <stdio.h>
#include <stdlib.h>
#include "threads.h"

typedef pthread_mutex_t lock_t;

lock_t L1;

void init_lock(lock_t *l)
{
      pthread_mutex_init(l, NULL);
}

void lock(lock_t *l)
{
       pthread_mutex_lock (l); 
}

void unlock (lock_t *l)
{
       pthread_mutex_unlock (l); pthread_yield();
}


void function_1(void)
{
    while (1){ 
        lock(&L1);
        printf("Beginning of CS: func 1\n");
        sleep(1);
        printf("End of CCS: func 1..\n");
        sleep(1);
	unlock(&L1);
    }
}    

void function_2(void)
{
    while (1){ 
        lock(&L1);
        printf("Beginning of CS: func 2\n");
        sleep(1);
        printf("End of CCS: func 2..\n");
        sleep(1);
	unlock(&L1);
    }
}    

void function_3(void)
{
    while (1){ 
       lock(&L1);
        printf("Beginning of CS: func 3\n");
        sleep(1);
        printf("End of CCS: func 3..\n");
        sleep(1);
	unlock(&L1);
    }
}    

int main()
{
    init_lock(&L1); 
    start_thread(function_1, NULL);
    start_thread(function_2, NULL);
    start_thread(function_3, NULL);

    while(1) sleep(1);

    return 0;
}




