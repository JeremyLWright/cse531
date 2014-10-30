// (c) Partha Dasgupta 2009
// permission to use and distribute granted.

#include "threads.h"

typedef pthread_mutex_t monitor_t;
typedef pthread_cond_t monitor_cond_t;

void init_monitor(monitor_t *M)
{   pthread_mutex_init(M, NULL);}

void init_monitor_cond(monitor_cond_t *monitor_cond)
{   pthread_cond_init(monitor_cond, NULL);}

void monitor_entry(monitor_t *M)
{   pthread_mutex_lock (M); }

void monitor_exit(monitor_t *M)
{    pthread_mutex_unlock (M); 
     pthread_yield();}

void monitor_wait(monitor_t *M, monitor_cond_t *monitor_cond)
{ pthread_cond_wait(monitor_cond, M); pthread_yield(); }

void monitor_signal(monitor_t *M, monitor_cond_t *monitor_cond)
{ pthread_cond_signal(monitor_cond); 
  pthread_yield();
}



