// dynbuf.c implementataion Generic dynamic byte buffer.
 

//#include "dynbuf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int dynbuf_init(DynBuf *buf, size_t initial_capacity) {
    buf->data = malloc(initial_capacity);
    if (!buf->data) {
        fprintf(stderr, "[dynbuf] malloc failed\n");
        return -1;
    }
    buf->size     = 0;
    buf->capacity = initial_capacity;
    return 0;
}

int dynbuf_reserve(DynBuf *buf, size_t capacity) {
    if (capacity <= buf->capacity) return 0;

    /* Double until large enough */
    size_t new_cap = buf->capacity ? buf->capacity : 64;
    while (new_cap < capacity)
        new_cap *= 2;

    unsigned char *tmp = realloc(buf->data, new_cap);
    if (!tmp) {
        fprintf(stderr, "[dynbuf] realloc failed\n");
        return -1;
    }
    buf->data     = tmp;
    buf->capacity = new_cap;
    return 0;
}

int dynbuf_append(DynBuf *buf, const unsigned char *src, size_t len) {
    if (dynbuf_reserve(buf, buf->size + len) != 0)
        return -1;
    memcpy(buf->data + buf->size, src, len);
    buf->size += len;
    return 0;
}

void dynbuf_reset(DynBuf *buf) {
    buf->size = 0;
}

void dynbuf_free(DynBuf *buf) {
    free(buf->data);
    buf->data     = NULL;
    buf->size     = buf->capacity = 0;
}
