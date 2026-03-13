#include <stdio.h>
#include <stdlib.h>
#include "chunker.h"

#define CHUNK_SIZE (1024 * 1024)

int chunk_file(const char *filename, Chunk **chunks, int *chunk_count) {
    FILE *file = fopen(filename, "rb");
    if(!file){
        fprintf(stderr, "Error could not open file '%s'\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    
    if (file_size <= 0){
        fprintf(stderr, "Error: file is empty or unreadable\n");
        fclose(file);
        return -1;
    }

    *chunk_count = (int)((file_size + CHUNK_SIZE - 1) / CHUNK_SIZE);

    *chunks = (Chunk *)malloc(sizeof(Chunk) * (*chunk_count));
    if (!*chunks){
        fprintf(stderr, "Error: memory allocation failed\n");
        fclose(file);
        return -1;
    }

    return 0;
}

void free_chunks(Chunk *chunks, int chunk_count) {
    // TODO: Shaheer — free all chunk data buffers
}