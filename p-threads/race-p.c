// (c) Partha Dasgupta 2009
// permission to use and distribute granted.


#include <stdio.h>
#include <stdlib.h>
#include "threads.h"

int x;

func1()
{int i = 0;
   printf("Thread 1 - running\n");
   for (i=0; i<100000000; i++) x++;  
   printf("Thread 1: x = %d\n", x); 
   printf("Thread 1 - done\n");
}

func2(void* arg)
{int i = 0;
   printf("      Thread 2 - running\n");
   for (i=0; i<100000000; i++) x++;
   printf("      Thread 2: x = %d\n", x); 
   printf("      Thread 2 - done\n");
}

main()
{
   printf("starting\n");
   pthread_t pid = start_thread(func2, NULL);
   printf("cloned: %ld\n", pid);
   func1();
   sleep(1);
   printf("final value %d\n", x);
}


