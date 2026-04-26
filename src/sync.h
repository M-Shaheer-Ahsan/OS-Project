#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>
#include <semaphore.h>

#define MAX_THREADS 8 // Change this to allow more parallel CPU usage!

extern sem_t            sem_slots;
extern pthread_mutex_t  output_mutex;
extern pthread_mutex_t  stats_mutex;
extern pthread_mutex_t  pause_mutex;
extern pthread_cond_t   pause_cond;
extern volatile int     is_paused;

int sync_init(void);
void sync_destroy(void);

#endif