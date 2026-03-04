#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>
#include <semaphore.h>

extern sem_t compression_slots;
extern pthread_mutex_t output_mutex;
extern pthread_mutex_t stats_mutex;

void sync_init(int max_threads);
void sync_destroy();

#endif