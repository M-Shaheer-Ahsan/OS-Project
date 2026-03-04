#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "compressor.h"
#include "sync.h"

void compress_chunks(Chunk *chunks, int chunk_count, int max_threads) {
    // TODO: Haziq — spawn threads, semaphore throttle, zlib compression
}