#include <stdio.h>
#include <stdlib.h>
#include "chunker.h"

#define CHUNK_SIZE (1024 * 1024 * 4)

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

    for (int i = 0; i < *chunk_count; i++) {
        size_t bytes_to_read = CHUNK_SIZE;
        if (i == *chunk_count - 1)
            bytes_to_read = file_size - ((long)i * CHUNK_SIZE);

        (*chunks)[i].data = (unsigned char *)malloc(bytes_to_read);
        if (!(*chunks)[i].data) {
            fprintf(stderr, "Error: memory allocation failed for chunk %d\n", i);
            for (int j = 0; j < i; j++) free((*chunks)[j].data);
            free(*chunks);
            fclose(file);
            return -1;
        }

        size_t bytes_read = fread((*chunks)[i].data, 1, bytes_to_read, file);
        if (bytes_read != bytes_to_read) {
            fprintf(stderr, "Error: could not read chunk %d\n", i);
            for (int j = 0; j <= i; j++) free((*chunks)[j].data);
            free(*chunks);
            fclose(file);
            return -1;
        }

        (*chunks)[i].id = i;
        (*chunks)[i].original_size = bytes_read;
        (*chunks)[i].compressed_size = 0;
        (*chunks)[i].priority = i;
    }

    fclose(file);
    printf("File split into %d chunk(s) of up to 1MB each\n", *chunk_count);

    return 0;
}

void free_chunks(Chunk *chunks, int chunk_count) {
    if (!chunks) return;
    for (int i = 0; i < chunk_count; i++) {
        if (chunks[i].data) {
            free(chunks[i].data);
            chunks[i].data = NULL;
        }
    }
    free(chunks);
}