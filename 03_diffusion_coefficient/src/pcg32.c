/*============================================================================
 * PCG32 RANDOM NUMBER GENERATOR
 * 
 * Minimal implementation of PCG32 (Permuted Congruential Generator)
 * Licensed under Apache License 2.0 - (c) 2014 M.E. O'Neill
 * Website: https://www.pcg-random.org/
 *===========================================================================*/
#include<math.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include"../include/pcg32.h"
/**
 * @brief Generate next random number from PCG32 generator
 * 
 * @param rng Pointer to PCG32 state structure
 * @return uint32_t Uniformly distributed 32-bit random integer
 * 
 * Uses XSH-RR (xorshift high, random rotation) output transformation
 * for excellent statistical properties. The generator has a period of 2^64.
 */
uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    
    // Advance internal state using LCG formula
    // state = state * multiplier + increment
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = (uint32_t) ( ((oldstate >> 18u) ^ oldstate) >> 27u );
    uint32_t rot = (uint32_t) ( oldstate >> 59u );
    
    // Apply random rotation for better output distribution
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

/**
 * @brief Initialize PCG32 generator with seed values
 * 
 * @param rng Pointer to PCG32 state structure to initialize
 * @param initstate Initial state (first seed)
 * @param initseq Sequence selector (second seed, determines stream)
 * 
 * Proper seeding procedure ensures that different seed pairs produce
 * independent random sequences. The initseq determines which of 2^63
 * possible streams is selected.
 */
void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;  // Ensure increment is odd
    pcg32_random_r(rng);               // Warm-up step
    rng->state += initstate;
    pcg32_random_r(rng);               // Second warm-up step
}

// Global PCG32 state for random walk generation
pcg32_random_t pcg32_random_state;

/**
 * @brief Initialize the random number generator for random walks
 * 
 * @param initstate First seed value
 * @param initseq Second seed value (sequence selector)
 * 
 * Wrapper function to initialize the global RNG state.
 * Call this with fresh seeds for each independent random walk.
 */
void myrand_init(unsigned long int initstate, unsigned long int initseq)
{
    pcg32_srandom_r(&pcg32_random_state, (uint64_t) initstate, (uint64_t) initseq);
}

/**
 * @brief Generate uniform random number in [0,1)
 * 
 * @return double Uniformly distributed random number in [0, 1)
 * 
 * Converts the 32-bit integer output from PCG32 to a double-precision
 * floating-point number in the range [0, 1). The division ensures
 * that 1.0 is never returned (half-open interval).
 */
double myrand(void)
{
    return (double) pcg32_random_r(&pcg32_random_state)/((double)UINT32_MAX + 1.0);
}
