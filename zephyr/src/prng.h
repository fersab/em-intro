#ifndef PRNG_H
#define PRNG_H

#include <stdint.h>

// Knuth LCG constants
#define LCG_MUL 1664525u
#define LCG_INC 1013904223u

// advance LCG state and return upper 16 bits
static inline uint32_t lcg_next(uint32_t *state)
{
    *state = *state * LCG_MUL + LCG_INC;
    return *state >> 16;
}

#endif
