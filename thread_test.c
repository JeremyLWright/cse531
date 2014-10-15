/*******************************************************************************
 * FILENAME:    thread_test.c
 * DESCRIPTION: Testing program for user-space threads: threads.h, q.h, TCB.h
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Distributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#include "threads.h"
#include <unistd.h> // sleep()
#include <stdlib.h> // rand(), srand()
#include <time.h> // time()

size_t alive = 0;
int globalInt = 42;

int randint(int max) { return (int)rand()/(RAND_MAX*1.0)*max; } 

void initrand() { srand((unsigned)(time(0))); }

int getNextChar(int myChar){
    // myChar cycles through the ASCII table from !(myChar=33) to ~(myChar=126)
    myChar = (myChar+1) % 127; 
    if (myChar < 33) {myChar = 33;};
    return myChar;
}

void func1(void)
{
    int i =0;
    int myValue = randint(1000);
    size_t my_id = alive++;
    printf("func%lu: started\n", my_id);
    while(1)
    { 
        printf("[%d] ", ++i);
        ++globalInt;
        printf("func%lu: swapcontext, myValue=%d, globalInt=%d\n", my_id, myValue, globalInt);
        yield();
        printf("func%lu: returning. myValue=%d\n\n", my_id, myValue);
        usleep(500000);
    }
    // this should never be reached - code is here for future compatibility
    printf("func%lu: terminating -  myValue=%d, globalInt=%d\n", my_id, myValue, globalInt);
    yield();
}

void func2(void)
{
    static int myChar = 0;
    myChar = randint(127);
    int j = 0;
    printf("func2B: started\n");
    while(1)
    {   
        myChar = getNextChar(myChar);
        printf("[%d] ", ++j);
        printf("func2B: swapcontext, myChar = '%c', globalInt = %d\n", myChar, globalInt);
        yield();
        printf("func2B: returning\n");
        usleep(500000);
    }
    // this should never be reached
}

void func3(void)
{
    int j = 0;
    printf("func3B: started\n");
    while(1)
    {
        printf("[%d] ", ++j);
        printf("func3B: swapcontext\n");
        yield();
        printf("func3B: returning\n\n");
        usleep(500000);
    }
    // this should never be reached
}

void func4(void)
{
    int j = 0;
    printf("func4B: started\n");
    while(1)
    {
        printf("[%d] ", ++j);
        printf("func4B: swapcontext\n");
        yield();
        printf("func4B: returning\n");
        usleep(500000);
    }
    // this should never be reached
}


int main(int argc, char *argv[])
{
    int i = 0;
    int totalThreads = 10;
    if (argc > 1){
        totalThreads = atoi(argv[1]);
    }
    printf("Begin Main function\n");
    printf("Main function: spawning %d child threads...\n", totalThreads);
    InitQ(&RunQ);
    initrand();
    start_thread(func2);
    start_thread(func3);
    start_thread(func4);
    for (i = 0; i < totalThreads; i++) {
        start_thread(func1);
    }
 
#if 0
    start_thread(func1);
    start_thread(func1);
    start_thread(func1);
    start_thread(func1);
    start_thread(func2);
    start_thread(func3);
    start_thread(func4);
#endif

    // this should never be reached
    printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
    run();
    
    printf("main: exiting\n");
    exit(EXIT_SUCCESS);
}
