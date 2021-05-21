#include "ring.h"


void ring_init(ring_t* ring, ring_size_t size) {
    ring->top = (char*)malloc(size);
    ring->end = ring->top + size;
    ring_clear(ring);
}



void ring_free(ring_t* ring) {
    free(ring->top);

    ring->top = ring->end = ring->in = ring->out = 0;
}


void ring_clear(ring_t* ring) {
    ring->in = ring->top;
    ring->out = ring->top;
}
