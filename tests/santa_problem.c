#include "sem.h"
#include "unistd.h"

//Problem is inspired bu Operatins Systems: Internals and Design Principles

semaphore_t santaSem;
semaphore_t mutex;
semaphore_t reindeerSem;
semaphore_t elfMutex;
size_t elves = 0;
size_t reindeers = 0;
void prepareSleigh()
{
    printf(ANSI_COLOR_YELLOW "\t\t%s\n" ANSI_COLOR_RESET, __func__);
}
void helpElves()
{
    printf(ANSI_COLOR_GREEN "\t\t%s\n" ANSI_COLOR_RESET, __func__);
    sleep(1);
}
void getHelp()
{
    printf(ANSI_COLOR_RED "\t\t%s\n" ANSI_COLOR_RESET, __func__);
    sleep(1);
}
void getHitched()
{
    printf(ANSI_COLOR_CYAN "\t\t%s\n" ANSI_COLOR_RESET, __func__);
    sleep(1);
}

void santa(void)
{
    int i = 0;
    while(1)
    {
        P(&mutex);
        if(reindeers == 9)
        {
            prepareSleigh();
            for(i = 0; i < 9; ++i)
                V(&reindeerSem);
        }
        else if(elves == 3)
        {
            helpElves();
        }
        V(&mutex);
        sleep(1);
    }
}

void reindeer(void)
{
    while(1)
    {
        P(&mutex);
        reindeers++;
        if(reindeers == 9)
        {
            V(&santaSem);
        }
        V(&mutex);
        P(&reindeerSem);
        getHitched();
    }
}

void elf(void)
{
    while(1)
    {
        P(&elfMutex);
        P(&mutex);
        {
            elves++;
            if(elves == 3)
                P(&santaSem);
            else
                V(&elfMutex);
        }
        V(&mutex);
        getHelp();
        P(&mutex);
        elves--;
        if(elves == 0)
            V(&elfMutex);
        V(&mutex);
    }
}

void init_seq_01(void)
{
    int i;
    for(i = 0; i < 9; ++i) start_thread(reindeer);
    for(i = 0; i < 100; ++i) start_thread(elf);
    start_thread(santa);
}
void init_seq_02(void)
{
    int i;
    for( i = 0; i < 100; ++i) start_thread(elf);
    start_thread(santa);
    for( i = 0; i < 9; ++i) start_thread(reindeer);
}

int main(int argc, const char *argv[])
{
    init_sem(&santaSem, 0);
    init_sem(&reindeerSem, 0);
    init_sem(&mutex, 1);
    init_sem(&elfMutex, 1);

    init_seq_01();

    run();


    return 0;
}
