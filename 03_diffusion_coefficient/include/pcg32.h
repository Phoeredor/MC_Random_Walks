#ifndef PCG32_H
#define PCG32_H

#include <stdint.h>

/**
 * @brief PCG32 random number generator state structure
 * 
 * This structure holds the internal state of a PCG32 generator.
 * - state: The main RNG state (64-bit)
 * - inc: The stream selector (must be odd, determines the sequence)
 */
typedef struct {
    uint64_t state;  // Current state of the generator
    uint64_t inc;    // Increment (stream identifier), always odd
} pcg32_random_t;

// Stato globale definito in pcg32.c
extern pcg32_random_t pcg32_random_state;

// Funzioni core PCG32
uint32_t pcg32_random_r(pcg32_random_t *rng);
void pcg32_srandom_r(pcg32_random_t *rng, uint64_t initstate, uint64_t initseq);

// Interfaccia high-level che usi nel tuo programma
void myrand_init(unsigned long int initstate, unsigned long int initseq);
double myrand(void);

#endif
