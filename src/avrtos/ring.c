#include "ring.h"

int8_t k_ring_init(struct k_ring *ring, uint8_t *buffer, uint8_t size)
{
	Z_ARGS_CHECK(ring && buffer && size) return -EINVAL;

	ring->buffer = buffer;
	ring->size	 = size;
	ring->r		 = 0u;
	ring->w		 = 0u;

	return 0;
}

int8_t k_ring_push(struct k_ring *ring, char data)
{
	Z_ARGS_CHECK(ring) return -EINVAL;

	const uint8_t r	  = ring->r;
	uint8_t w		  = ring->w;
	const uint8_t rem = ring->size - (w - r) - 1u;

	if (!rem) {
		return -ENOMEM;
	}

	ring->buffer[w] = data;

	w++;

	if (w == ring->size) {
		w = 0u;
	}

	ring->w = w;

	return 0;
}

int8_t k_ring_pop(struct k_ring *ring, char *data)
{
	Z_ARGS_CHECK(ring && data) return -EINVAL;

	const uint8_t w = ring->w;
	uint8_t r		= ring->r;

	if (r == w) {
		return -EAGAIN;
	}

	*data = ring->buffer[r];

	r++;

	if (r == ring->size) {
		r = 0u;
	}

	ring->r = r;

	return 0;
}

int8_t k_ring_reset(struct k_ring *ring)
{
	ARG_UNUSED(ring);
	
	return -ENOTSUP;
}