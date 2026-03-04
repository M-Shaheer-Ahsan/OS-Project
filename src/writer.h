#ifndef WRITER_H
#define WRITER_H

#include "chunk.h"

int write_output(const char *filename, Chunk *chunks, int chunk_count, size_t original_size);

#endif