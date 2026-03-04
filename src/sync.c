#include "sync.h"

sem_t compression_slots;
pthread_mutex_t output_mutex;
pthread_mutex_t stats_mutex;

void sync_init(int max_threads) {
    // TODO: Haziq — initialize semaphore and mutexes
}

void sync_destroy() {
    // TODO: Haziq — destroy semaphore and mutexes
}