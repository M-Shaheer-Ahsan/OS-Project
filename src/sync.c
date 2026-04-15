#include "sync.h"
#include <stdio.h>

sem_t            sem_slots;
pthread_mutex_t  output_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  stats_mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  pause_mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   pause_cond   = PTHREAD_COND_INITIALIZER;
volatile int     is_paused    = 0;

int sync_init(void) {
    if (sem_init(&sem_slots, 0, MAX_THREADS) != 0) {
        perror("[sync] sem_init failed");
        return -1;
    }
    return 0;
}

void sync_destroy(void) {
    sem_destroy(&sem_slots);
    pthread_mutex_destroy(&output_mutex);
    pthread_mutex_destroy(&stats_mutex);
    pthread_mutex_destroy(&pause_mutex);
    pthread_cond_destroy(&pause_cond);
}
