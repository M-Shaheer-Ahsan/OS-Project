#include "compressor.h"
#include "sync.h"
#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zlib.h>
#include <unistd.h>

ProgressStats g_stats = {0};


static Chunk         **pq_heap = NULL;
static int             pq_size = 0;
static int             pq_cap  = 0;

static void pq_swap(int i, int j) {
    Chunk *tmp  = pq_heap[i];
    pq_heap[i]  = pq_heap[j];
    pq_heap[j]  = tmp;
}

static void pq_bubble_up(int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq_heap[parent]->priority <= pq_heap[i]->priority) break;
        pq_swap(i, parent);
        i = parent;
    }
}

static void pq_bubble_down(int i) {
    while (1) {
        int smallest = i;
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        if (left  < pq_size && pq_heap[left]->priority  < pq_heap[smallest]->priority) smallest = left;
        if (right < pq_size && pq_heap[right]->priority < pq_heap[smallest]->priority) smallest = right;
        if (smallest == i) break;
        pq_swap(i, smallest);
        i = smallest;
    }
}

static int pq_push(Chunk *c) {
    if (pq_size == pq_cap) {
        int new_cap  = pq_cap ? pq_cap * 2 : 8;
        Chunk **tmp  = realloc(pq_heap, new_cap * sizeof(Chunk *));
        if (!tmp) return -1;
        pq_heap = tmp;
        pq_cap  = new_cap;
    }
    pq_heap[pq_size++] = c;
    pq_bubble_up(pq_size - 1);
    return 0;
}

static Chunk *pq_pop(void) {
    if (pq_size == 0) return NULL;
    Chunk *top  = pq_heap[0];
    pq_heap[0]  = pq_heap[--pq_size];
    pq_bubble_down(0);
    return top;
}

static void pq_free(void) {
    free(pq_heap);
    pq_heap = NULL;
    pq_size = pq_cap = 0;
}


typedef struct {
    Chunk *chunk;
    int    total;
} WorkerArgs;

static void *compress_worker(void *arg) {
    WorkerArgs *wa    = (WorkerArgs *)arg;
    Chunk      *chunk = wa->chunk;
    int         total = wa->total;
    free(wa);

    pthread_mutex_lock(&pause_mutex);
    while (is_paused)
        pthread_cond_wait(&pause_cond, &pause_mutex);
    pthread_mutex_unlock(&pause_mutex);

    sem_wait(&sem_slots);

    int my_slot = -1;
    pthread_mutex_lock(&stats_mutex);
    for (int i = 0; i < 8; i++) {
        if (g_stats.active_slots[i] == 0) {
            g_stats.active_slots[i] = chunk->id + 1; // Mark as BUSY
            my_slot = i;
            break;
        }
    }
    pthread_mutex_unlock(&stats_mutex);

    usleep(500000);

    uLongf bound = compressBound((uLong)chunk->original_size);
    unsigned char *out = malloc(bound);
    if (!out) {
        fprintf(stderr, "[compressor] malloc failed for chunk %d\n", chunk->id);
        if (my_slot >= 0) {
            pthread_mutex_lock(&stats_mutex);
            g_stats.active_slots[my_slot] = 0;
            pthread_mutex_unlock(&stats_mutex);
        }
        sem_post(&sem_slots);
        return NULL;
    }

    uLongf compressed_len = bound;
    int ret = compress2(out, &compressed_len,
                        chunk->data, (uLong)chunk->original_size,
                        Z_BEST_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "[compressor] compress2 failed for chunk %d: %s\n",
                chunk->id, zError(ret));
        free(out);
        if (my_slot >= 0) {
            pthread_mutex_lock(&stats_mutex);
            g_stats.active_slots[my_slot] = 0;
            pthread_mutex_unlock(&stats_mutex);
        }
        sem_post(&sem_slots);
        return NULL;
    }

    free(chunk->data);
    chunk->data            = out;
    chunk->compressed_size = (size_t)compressed_len;

    pthread_mutex_lock(&stats_mutex);
    if (my_slot >= 0) {
        g_stats.active_slots[my_slot] = 0; // Set back to IDLE
    }
    
    g_stats.chunks_done++;
    g_stats.total_original_bytes   += chunk->original_size;
    g_stats.total_compressed_bytes += chunk->compressed_size;
    g_stats.progress_pct = (total > 0)
        ? (int)((g_stats.chunks_done * 100) / total)
        : 100;
    pthread_mutex_unlock(&stats_mutex);

    sem_post(&sem_slots);
    return NULL;
}


int compress_chunks(Chunk *chunks, int count) {
    if (!chunks || count <= 0) return -1;

    for (int i = 0; i < count; i++) {
        if (pq_push(&chunks[i]) != 0) {
            fprintf(stderr, "[compressor] pq_push failed\n");
            return -1;
        }
    }

    pthread_t *threads = malloc(count * sizeof(pthread_t));
    if (!threads) { perror("[compressor] malloc threads"); return -1; }

    g_stats.total_chunks = count;

    for (int i = 0; i < count; i++) {
        Chunk *c = pq_pop();
        if (!c) break;

        WorkerArgs *wa = malloc(sizeof(WorkerArgs));
        if (!wa) { perror("[compressor] malloc WorkerArgs"); break; }
        wa->chunk = c;
        wa->total = count;

        if (pthread_create(&threads[i], NULL, compress_worker, wa) != 0) {
            perror("[compressor] pthread_create");
            free(wa);
        }
    }

    for (int i = 0; i < count; i++)
        pthread_join(threads[i], NULL);

    free(threads);
    pq_free();
    return 0;
}

void compressor_pause(void) {
    pthread_mutex_lock(&pause_mutex);
    is_paused = 1;
    pthread_mutex_unlock(&pause_mutex);
}

void compressor_resume(void) {
    pthread_mutex_lock(&pause_mutex);
    is_paused = 0;
    pthread_cond_broadcast(&pause_cond);
    pthread_mutex_unlock(&pause_mutex);
}

ProgressStats compressor_get_stats(void) {
    pthread_mutex_lock(&stats_mutex);
    ProgressStats snap = g_stats;
    pthread_mutex_unlock(&stats_mutex);
    return snap;
}
