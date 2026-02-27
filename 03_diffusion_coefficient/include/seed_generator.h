/**
 * @file seed_generator.h
 * @brief Header file for PCG32-based seed generation
 * 
 * This module provides a wrapper around the PCG32 random number generator
 * to generate high-quality seeds for other random number generators.
 * Each call to generate_seed() produces a new 32-bit unsigned integer
 * suitable for seeding independent random walks or simulations.
 */

#ifndef SEED_GENERATOR_H
#define SEED_GENERATOR_H

#include <stdint.h>
#include "pcg32.h"  // Include pcg32.h to use pcg32_random_t type

/**
 * @brief Initialize the PCG32 generator with custom seed values
 * 
 * @param initstate Initial state value (64-bit seed)
 * @param initseq Sequence selector (determines which random sequence to use)
 * 
 * This function seeds the internal PCG32 generator. Different initseq values
 * produce independent random streams even with the same initstate.
 */
void pcg32_srandom(uint64_t initstate, uint64_t initseq);

/**
 * @brief Generate a uniformly distributed 32-bit random number
 * 
 * @return uint32_t Random number in the full range [0, 2^32-1]
 * 
 * This is the core PCG32 generation function using XSH-RR output transformation.
 */
uint32_t pcg32_random(void);

/**
 * @brief Initialize the seed generator with specified state and sequence
 * 
 * @param state Initial state for the seed generator
 * @param seq Sequence number for the seed generator
 * 
 * This should be called once at program startup to initialize the global
 * seed generator. Use fixed values for reproducibility or time-based values
 * for non-deterministic behavior.
 */
void seedgen_init(uint64_t state, uint64_t seq);

/**
 * @brief Generate a new random seed
 * 
 * @return unsigned int A 32-bit random seed suitable for initializing RNGs
 * 
 * This function is called to obtain fresh seeds for each independent random walk.
 * Each call advances the internal state and returns a new random value.
 */
unsigned int generate_seed(void);

/**
 * @brief Test utility to print n generated seeds
 * 
 * @param n Number of seeds to generate and print
 * 
 * Utility function for testing and verification purposes.
 */
void test_seeds(int n);

#endif // SEED_GENERATOR_H
