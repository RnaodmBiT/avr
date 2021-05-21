#ifndef _RING_H_
#define _RING_H_

#include <system.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint8_t ring_size_t;

typedef volatile struct {
    char* in;   // Pointer to next element to write.
    char* out;  // Pointer to next element to read.
    char* top;  // Pointer to top of buffer.
    char* end;  // Pointer to char after buffer end.
} ring_t;

#define RING_SIZE(RING) \
    ((RING)->end - (RING)->top)

#define RING_READ_NUM(RING, TMP) \
    (((TMP) = ((RING)->in - (RING)->out)) < 0 \
     ? (TMP) + RING_SIZE(RING) : (TMP))

#define RING_WRITE_NUM(RING, TMP) \
    (RING_SIZE(RING) - RING_READ_NUM(RING, TMP) - 1)


// Initialize a buffer with the requested block of memory. If there is not
// enough memory, the buffer is set to zero size
void ring_init(ring_t* ring, ring_size_t size);


// Free the memory from a circle buffer and set it to size zero
void ring_free(ring_t* ring);


// Clear all data from the ring buffer.
void ring_clear(ring_t* ring);


inline ring_size_t ring_read_num(ring_t* ring) {
    int16_t tmp;
    return RING_READ_NUM(ring, tmp);
}


inline ring_size_t ring_write_num(ring_t* ring) {
    uint16_t tmp;
    return RING_WRITE_NUM(ring, tmp);
}


inline bool ring_is_empty(ring_t* ring) {
    return ring_read_num(ring) == 0;
}


inline bool ring_is_full(ring_t* ring) {
    return ring_write_num(ring) == 0;
}


// Read a character from the ring buffer. Returns -1 on no data
inline char ring_getc(ring_t* ring) {
    char c;

    if (ring_is_empty(ring)) {
        return -1;
    }

    char* ptr = ring->out;
    c = *ptr++;
    if (ptr >= ring->end) {
        ptr = ring->top;
    }

    ring->out = ptr;

    return c;
}


// Warning: this defaults to overwrite old data
inline void ring_putc(ring_t* ring, char c) {
    if (ring_is_full(ring)) {
        ring_getc(ring); // read a character out so we can fit a new one in
    }

    char* ptr = ring->in;
    *ptr++ = c;

    if (ptr >= ring->end) {
        ptr = ring->top;
    }

    ring->in = ptr;
}



#if defined(__cplusplus)
}
#endif

#endif // _RING_H_
