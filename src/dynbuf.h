#ifndef DYNBUF_H
#define DYNBUF_H

#include <stddef.h>

typedef struct {
    unsigned char *data;
    size_t         size;      /* bytes currently used   */
    size_t         capacity;  /* bytes allocated        */
} DynBuf;

/* Initialise an empty buffer with a given initial capacity */
int  dynbuf_init(DynBuf *buf, size_t initial_capacity);

/* Append bytes to the buffer, growing if necessary */
int  dynbuf_append(DynBuf *buf, const unsigned char *src, size_t len);

/* Ensure at least `capacity` bytes are allocated */
int  dynbuf_reserve(DynBuf *buf, size_t capacity);

/* Reset used size to 0 without freeing memory (reuse the buffer) */
void dynbuf_reset(DynBuf *buf);

/* Free all memory */
void dynbuf_free(DynBuf *buf);

#endif /* DYNBUF_H */