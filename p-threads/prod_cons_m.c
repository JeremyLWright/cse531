// (c) Partha Dasgupta 2009
// permission to use and distribute granted.

#include <stdio.h>
#include <stdlib.h>
#include "monitor.h"

#define N 4

monitor_t buff_mon;
monitor_cond_t empty, full;

int buffer[N];
int in = 0, out = 0, count=0, item_num=0, prod_delay = 1, cons_delay = 1;




void prod (int *arg)
{
    while (1){ 
	printf("Producer %d: ready to produce\n", *arg);
        monitor_entry(&buff_mon);
          if (count>=N) 
            monitor_wait(&buff_mon, &empty);
	    printf("Producer %d: inserting item#%d, into slot #%d\n", *arg, item_num, in);
          buffer[in] = item_num++; in = (in+1) % N; count++;
	    monitor_signal(&buff_mon, &full);
        monitor_exit(&buff_mon);
	sleep(prod_delay);
    }
}    

void cons(int *arg)
{
    while(1){
	printf("        Consumer %d: ready to consume\n", *arg);
        monitor_entry(&buff_mon);
	  if (count<=0) 
            monitor_wait(&buff_mon, &full);
  	  printf("       Consumer %d: deleting item#%d, from slot #%d\n", *arg, buffer[out], out);
          out = (out+1) % N; count--;
	    monitor_signal(&buff_mon, &empty);
        monitor_exit(&buff_mon);
        sleep(cons_delay);
    }    
}


int main()
{
    int id[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    init_monitor(&buff_mon);
    init_monitor_cond(&empty);
    init_monitor_cond(&full);

    start_thread(prod, &id[0]);
    start_thread(cons, &id[1]);
    start_thread(prod, &id[2]);
    start_thread(cons, &id[3]);
    start_thread(prod, &id[4]);
    start_thread(cons, &id[5]);
    start_thread(prod, &id[6]);
    start_thread(cons, &id[7]);
    while (1) { scanf("%d %d", &prod_delay, &cons_delay); 
                printf ("\n\n\t\t\t\tP=%d C=%d\n\n\n", prod_delay, cons_delay);
    };
}




