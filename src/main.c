#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "chunker.h"
#include "compressor.h"
#include "writer.h"
#include "decompressor.h"
#include "sync.h"
#include "gui.h"

// Wrapper to pass arguments to our background compression thread
typedef struct {
    Chunk *chunks;
    int count;
} CompressArgs;

void* compression_thread_func(void* arg) {
    CompressArgs* ca = (CompressArgs*)arg;
    compress_chunks(ca->chunks, ca->count);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage:\n");
        printf("  Compress:   compressor c <input> <output>\n");
        printf("  Decompress: compressor d <input> <output>\n");
        return 1;
    }

    char mode = argv[1][0];

    if (mode == 'd') {
        printf("Decompressing '%s' -> '%s'...\n", argv[2], argv[3]);
        return decompress_file(argv[2], argv[3]);
    }

    if (mode == 'c') {
        if (sync_init() != 0) return 1;

        Chunk *chunks = NULL;
        int chunk_count = 0;

        // 1. Chunk the file
        if (chunk_file(argv[2], &chunks, &chunk_count) != 0) {
            sync_destroy();
            return 1;
        }

        size_t original_size = 0;
        for (int i = 0; i < chunk_count; i++) {
            original_size += chunks[i].original_size;
        }

        // 2. Start the GUI
        gui_init();

        // 3. Launch Compression in a background thread
        CompressArgs ca = { chunks, chunk_count };
        pthread_t comp_thread;
        pthread_create(&comp_thread, NULL, compression_thread_func, &ca);

        // 4. Main GUI Render Loop
        while (1) {
            ProgressStats stats = compressor_get_stats();
            gui_update(stats.chunks_done, chunk_count);

            if (stats.chunks_done >= chunk_count) {
                break; // Everything is compressed!
            }
            usleep(50000); // 50ms refresh rate
        }

        // Wait for the background thread to officially close
        pthread_join(comp_thread, NULL);

        // 5. Write the compressed data to the final file
        write_output(argv[3], chunks, chunk_count, original_size);

        // 6. Keep the "100% Finished" GUI open until the user presses Q
        while (1) {
            gui_update(chunk_count, chunk_count);
            usleep(50000);
        }

        // Clean up (Note: gui.c handles exit(0) on 'Q', but this is good practice)
        gui_destroy();
        free_chunks(chunks, chunk_count);
        sync_destroy();
        return 0;
    }

    printf("Invalid mode '%c'. Use 'c' or 'd'.\n", mode);
    return 1;
}