// (c) Partha Dasgupta 2009
// permission to use and distribute granted.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "monitor.h"

monitor_t read_write;
monitor_cond_t r_cond, w_cond;

int rwc = 0, wwc = 0, rc = 0, wc = 0, global_ID=0;

void reader_entry(int ID)
{
	printf("[reader: #%d]\ttrying to read\n", ID);
	monitor_entry(&read_write);
	if (wwc > 0 || wc > 0) {
	    printf("[reader: #%d]\tblocking for writer\n", ID);
	    rwc++;		// increment waiting reader count.
            monitor_wait(&read_write, &r_cond);
	    rwc--;		
	}
	rc++;
	monitor_signal(&read_write, &r_cond);
	monitor_exit(&read_write);
}

void reader_exit(int ID)
{
	monitor_entry(&read_write);
	rc--;			// i'm no longer a reader
	if (rc==0) monitor_signal(&read_write, &w_cond);
        monitor_exit(&read_write);
}

void writer_entry(int ID)
{
	printf("\t\t\t\t[writer: #%d]\ttrying to write\n", ID);
	monitor_entry(&read_write);
	if (rc > 0 || wc > 0) {
	    printf("\t\t\t\t[writer: #%d] blocking for others\n");
	    wwc++;		// increment waiting writers
            monitor_wait(&read_write, &w_cond);
	    wwc--;		// i'm no longer waiting
	}
	wc++;			// increment writers
        monitor_exit(&read_write);
}

void writer_exit(int ID)
{
	monitor_entry(&read_write);
	wc--;
	if (rwc > 0) 		// first, i let all the readers go.
	    monitor_signal(&read_write, &r_cond);
	else 
	    monitor_signal(&read_write, &w_cond);
        monitor_exit(&read_write);
}



void reader(void)
{ 
  int ID;
  monitor_entry(&read_write); ID = global_ID++; monitor_exit(&read_write);
  while(1){
	reader_entry(ID);
	printf
	    ("[reader #%d]\t****READING****\n", ID);
	sleep(1);
	reader_exit(ID);
  };
}

void writer(void)
{
  int ID;
  monitor_entry(&read_write); ID = global_ID++; monitor_exit(&read_write);
  while(1){
 	writer_entry(ID);
	printf
	    ("\t\t\t\t[writer: #%d]\t&&&WRITING!&&&\n", ID);
	sleep(1);
	writer_exit(ID);
  };
}



//-------------------------------------------------------

int main()
{
    init_monitor(&read_write);
    init_monitor_cond(&r_cond);
    init_monitor_cond(&w_cond);

    start_thread(reader, NULL);
    start_thread(reader, NULL);
    start_thread(reader, NULL);
    start_thread(reader, NULL);
    start_thread(writer, NULL);
    start_thread(writer, NULL);
    start_thread(writer, NULL);

    while (1) sleep(1);
}   




