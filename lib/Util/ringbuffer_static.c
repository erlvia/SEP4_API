#include "ringbuffer_static.h"
#include <string.h>

static void advance_pointer(ringbuffer_t *rb) {
    if (rb->full) {
        rb->tail = (rb->tail + 1) % rb->capacity; // drop ældste
    }
    rb->head = (rb->head + 1) % rb->capacity;
    rb->full = (rb->head == rb->tail);
}

static void retreat_pointer(ringbuffer_t *rb) {
    rb->full = false;
    rb->tail = (rb->tail + 1) % rb->capacity;
}

bool ringbuffer_init_static(ringbuffer_t *rb,
                            void *storage,
                            size_t storage_bytes,
                            size_t capacity,
                            size_t elem_size)
{
    if (!rb || !storage || capacity == 0 || elem_size == 0) return false;

    size_t needed = capacity * elem_size;
    if (storage_bytes < needed) return false;

    rb->buffer   = (uint8_t *)storage;
    rb->capacity = capacity;
    rb->elem_size = elem_size;
    rb->head = 0;
    rb->tail = 0;
    rb->full = false;
    return true;
}

void ringbuffer_reset(ringbuffer_t *rb) {
    if (!rb) return;
    rb->head = 0;
    rb->tail = 0;
    rb->full = false;
}

bool ringbuffer_push(ringbuffer_t *rb, const void *item) {
    if (!rb || !rb->buffer || !item) return false;

    memcpy(rb->buffer + (rb->head * rb->elem_size),
           item,
           rb->elem_size);

    advance_pointer(rb);
    return true;
}

bool ringbuffer_pop(ringbuffer_t *rb, void *item) {
    if (!rb || !rb->buffer || !item) return false;
    if (ringbuffer_is_empty(rb)) return false;

    memcpy(item,
           rb->buffer + (rb->tail * rb->elem_size),
           rb->elem_size);

    retreat_pointer(rb);
    return true;
}

bool ringbuffer_is_empty(const ringbuffer_t *rb) {
    return (rb && !rb->full && (rb->head == rb->tail));
}

bool ringbuffer_is_full(const ringbuffer_t *rb) {
    return (rb && rb->full);
}

size_t ringbuffer_size(const ringbuffer_t *rb) {
    if (!rb) return 0;

    if (rb->full) return rb->capacity;

    if (rb->head >= rb->tail) return rb->head - rb->tail;

    return rb->capacity + rb->head - rb->tail;
}
