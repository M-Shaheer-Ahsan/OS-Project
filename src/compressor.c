#include "compressor.h"
#include "sync.h"
#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zlib.h>

/* ─── Global Progress Stats ─────────────────────────────────────────── */

ProgressStats g_stats = {0};

/* ─── Priority Queue (min-heap by chunk->priority) ──────────────────── */

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