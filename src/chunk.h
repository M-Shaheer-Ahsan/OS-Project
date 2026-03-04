#ifndef CHUNK_H
#define CHUNK_H

#include <stddef.h>

typedef struct {
    unsigned char *data;        // raw chunk bytes
    size_t original_size;       // size before compression
    size_t compressed_size;     // filled in after compression
    int id;                     // chunk index (0, 1, 2...)
    int priority;               // for priority queue
} Chunk;

#endif