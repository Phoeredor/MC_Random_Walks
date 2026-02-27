/**
 * @file seed_generator.c
 * @brief Implementation of PCG32-based seed generator
 * 
 * This module implements a seed generator using the PCG32 algorithm.
 * PCG (Permuted Congruential Generator) is a family of simple fast 
 * space-efficient statistically good algorithms for random number generation.
 * 
 * Reference: https://www.pcg-random.org/
 * Algorithm: PCG XSH-RR 64/32
 */

#include <stdint.h>
#include <stdio.h>
#include "seed_generator.h"

/**
 * @brief Global PCG32 state for seed generation
 * 
 * This static variable maintains the internal state of the seed generator.
 * It is initialized with arbitrary constants and should be re-seeded via
 * seedgen_init() at program startup for better randomness.
 */
static pcg32_random_t rng_state = { 
    0x853c49e6748fea9bULL,  // Initial state value
    0xda3e39cb94b95bdbULL   // Initial increment value
};

/**
 * @brief Seed the PCG32 generator
 * 
 * @param initstate Initial state (provides the starting point)
 * @param initseq Sequence selector (chooses which of 2^63 streams to use)
 * 
 * This function properly initializes the PCG32 generator following the
 * standard seeding procedure:
 * 1. Set state to 0 and inc to (initseq << 1) | 1 (ensuring inc is odd)
 * 2. Step the generator once (warm-up)
 * 3. Add initstate to the current state
 * 4. Step the generator again (second warm-up)
 */
void pcg32_srandom(uint64_t initstate, uint64_t initseq) {
    rng_state.state = 0U;
    rng_state.inc = (initseq << 1u) | 1u;  // Ensure inc is always odd
    pcg32_random();  // Warm up the generator
    rng_state.state += initstate;
    pcg32_random();  // Second warm-up step
}

/**
 * @brief Generate a 32-bit random number using PCG32 algorithm
 * 
 * @return uint32_t Uniformly distributed random integer in [0, 2^32-1]
 * 
 * PCG XSH-RR (xorshift high, random rotation) algorithm:
 * 1. Save the old state for output calculation
 * 2. Advance internal state using LCG: state = state * multiplier + increment
 * 3. Apply XSH-RR output function for better statistical properties:
 *    - XOR and shift operations on the old state
 *    - Random rotation based on high bits of old state
 * 
 * The output function ensures that even though the internal state follows
 * a simple LCG pattern, the output passes rigorous statistical tests.
 */
uint32_t pcg32_random(void) {
    uint64_t oldstate = rng_state.state;
    
    // Advance internal state (Linear Congruential Generator step)
    // Multiplier: 6364136223846793005 (proven good LCG constant)
    rng_state.state = oldstate * 6364136223846793005ULL + rng_state.inc;
    
    // Calculate output using XSH-RR (XorShift High, Random Rotation)
    // This transformation provides excellent statistical properties
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;  // Use top 5 bits for rotation amount
    
    // Apply random rotation to xorshifted value
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

/**
 * @brief Initialize the seed generator with chosen state and sequence
 * 
 * @param state Initial state value (use time or fixed value)
 * @param seq Sequence number (allows multiple independent streams)
 * 
 * This is the public interface to initialize the global seed generator.
 * Call this once at program startup. For reproducible results, use
 * fixed values. For non-deterministic behavior, use time-based values.
 */
void seedgen_init(uint64_t state, uint64_t seq) {
    pcg32_srandom(state, seq);
}

/**
 * @brief Generate a new random seed for use in simulations
 * 
 * @return unsigned int A 32-bit seed suitable for initializing other RNGs
 * 
 * This function is the main interface for obtaining seeds. Each call
 * advances the internal state and returns a fresh random value.
 * Use this to get independent seeds for each random walk simulation run.
 */
unsigned int generate_seed(void) {
    return pcg32_random();
}

/**
 * @brief Test utility function to print n generated seeds
 * 
 * @param n Number of seeds to generate and display
 * 
 * Useful for debugging and verifying that the seed generator is working
 * correctly. Prints seeds in a human-readable format.
 */
void test_seeds(int n) {
    for (int i = 0; i < n; ++i) {
        printf("Seed %d: %u\n", i + 1, generate_seed());
    }
}
