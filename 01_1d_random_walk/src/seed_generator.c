#include <stdint.h>
#include <stdio.h>
#include "../include/seed_generator.h"

// Global internal state of the PCG generator
static pcg32_random_t rng_state = { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL };

// Initialize the generator with two arbitrary 64-bit values
void pcg32_srandom(uint64_t initstate, uint64_t initseq) {
    rng_state.state = 0U;
    rng_state.inc = (initseq << 1u) | 1u;
    pcg32_random();  // warm up
    rng_state.state += initstate;
    pcg32_random();  // warm up again
}

// Generate a 32-bit uniformly distributed random integer
uint32_t pcg32_random(void) {
    uint64_t oldstate = rng_state.state;
    // Advance internal state
    rng_state.state = oldstate * 6364136223846793005ULL + rng_state.inc;
    // Calculate output function (XSH RR)
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// Optional: initialize the seed generator with chosen state and sequence
void seedgen_init(uint64_t state, uint64_t seq) {
    pcg32_srandom(state, seq);
}

// Return a new 32-bit unsigned seed
unsigned int generate_seed(void) {
    return pcg32_random();
}

// Simple test utility to print n generated seeds
void test_seeds(int n) {
    for (int i = 0; i < n; ++i) {
        printf("Seed %d: %u\n", i + 1, generate_seed());
    }
}

