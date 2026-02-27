/**
 * @file 2d_ran_walk.c
 * @brief 2D Lattice Random Walk Simulator with PCG32 Random Number Generator
 *
 * This program simulates 2D random walks on a square lattice with unit steps.
 * Each run uses independently seeded PCG32 generators to ensure statistical
 * independence. The program records position data at a specified target time
 * across multiple runs, enabling statistical analysis of the walk distribution.
 *
 * Features:
 * - Multiple independent simulation runs
 * - Configurable iterations per run
 * - Fixed-time sampling for statistical analysis
 * - Automatic mean and variance calculation
 * - High-quality PCG32 random number generation
 *
 * Output: Data file containing run number, time, step number, and x-position
 *         at the specified target time for each run.
 */

#include "../include/seed_generator.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // for getpid()

// Lattice step size (unit step in each direction)
#define lattice_step 1

/*============================================================================
 * PCG32 RANDOM NUMBER GENERATOR
 *
 * Minimal implementation of PCG32 (Permuted Congruential Generator)
 * Licensed under Apache License 2.0 - (c) 2014 M.E. O'Neill
 * Website: https://www.pcg-random.org/
 *===========================================================================*/

/**
 * @brief Generate next random number from PCG32 generator
 *
 * @param rng Pointer to PCG32 state structure
 * @return uint32_t Uniformly distributed 32-bit random integer
 *
 * Uses XSH-RR (xorshift high, random rotation) output transformation
 * for excellent statistical properties. The generator has a period of 2^64.
 */
uint32_t pcg32_random_r(pcg32_random_t *rng) {
  uint64_t oldstate = rng->state;

  // Advance internal state using LCG formula
  // state = state * multiplier + increment
  rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);

  // Calculate output function (XSH RR), uses old state for max ILP
  uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
  uint32_t rot = (uint32_t)(oldstate >> 59u);

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
void pcg32_srandom_r(pcg32_random_t *rng, uint64_t initstate,
                     uint64_t initseq) {
  rng->state = 0U;
  rng->inc = (initseq << 1u) | 1u; // Ensure increment is odd
  pcg32_random_r(rng);             // Warm-up step
  rng->state += initstate;
  pcg32_random_r(rng); // Second warm-up step
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
void myrand_init(unsigned long int initstate, unsigned long int initseq) {
  pcg32_srandom_r(&pcg32_random_state, (uint64_t)initstate, (uint64_t)initseq);
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
double myrand(void) {
  return (double)pcg32_random_r(&pcg32_random_state) /
         ((double)UINT32_MAX + 1.0);
}

/*============================================================================
 * STATISTICAL FUNCTIONS
 *===========================================================================*/

/**
 * @brief Calculate arithmetic mean
 *
 * @param dim Number of data points
 * @param sum_val Sum of all data points
 * @return double The arithmetic mean
 *
 * Simple mean calculation: mean = sum / n
 */
double mean_funct(size_t dim, double sum_val) {
  double mean = sum_val / ((double)dim);
  return mean;
}

/**
 * @brief Calculate sample variance
 *
 * @param dim Number of data points
 * @param arr Pointer to array of data values
 * @param avg Function pointer to mean calculation function
 * @return double Sample variance using (n-1) denominator
 *
 * Calculates unbiased sample variance: s² = Σ(x_i - mean)² / (n-1)
 * Uses Bessel's correction (n-1) for unbiased estimation.
 *
 * Note: This function signature expects a double** but is not used
 *       in the current implementation (variance is computed inline).
 */
double var_funct(size_t dim, double **arr, double (*avg)(size_t, double **)) {
  double sqrdev = 0.0;
  double mean = avg(dim, arr);

  // Sum squared deviations from mean
  for (size_t i = 0; i < dim; i++)
    sqrdev += ((*arr)[i] - mean) * ((*arr)[i] - mean);

  // Return sample variance (Bessel's correction: divide by n-1)
  return sqrdev / (dim - 1);
}

/*============================================================================
 * MAIN SIMULATION
 *===========================================================================*/

int main() {
  // Initialize the seed generator (produces seeds for individual runs)
  // Uses fixed values for reproducibility - change for different sequences
  seedgen_init(12345ULL, 67890ULL);

  // Simulation parameters
  int runs = 0;       // Number of independent random walk simulations
  int iterations = 0; // Number of steps per walk
  int t_target = 0;   // Target time for recording positions

  // Get number of runs from user
  printf("Enter number of runs: ");
  if (scanf("%d", &runs) != 1 || runs <= 0) {
    fprintf(stderr, "Invalid number of runs.\n");
    return EXIT_FAILURE;
  }

  // Get number of iterations (steps) per run
  printf("Enter number of iterations per run: ");
  if (scanf("%d", &iterations) != 1 || iterations <= 0) {
    fprintf(stderr, "Invalid number of iterations.\n");
    return EXIT_FAILURE;
  }

  // Get target time for position sampling
  printf("Enter number of time target: ");
  if (scanf("%d", &t_target) != 1 || t_target <= 0) {
    fprintf(stderr, "Invalid number of time target.\n");
    return EXIT_FAILURE;
  }

  /**
   * @struct strc
   * @brief Structure to represent a lattice point and walk state
   *
   * @var x X-coordinate on the lattice
   * @var y Y-coordinate on the lattice
   * @var step Current step number in the walk
   * @var time Current time (equivalent to step in this simulation)
   */
  typedef struct p {
    long int x, y;  // Position coordinates on 2D lattice
    int step, time; // Step counter and time variable
  } strc;

  // Initialize position at origin
  strc pos = {0, 0, 0, 0};

  // Accumulator for computing mean of x,y-positions at target time
  double sum_vals_x = 0.0, sum_vals_y = 0.0;

  /*========================================================================
   * MAIN SIMULATION LOOP - Execute multiple independent random walks
   *========================================================================*/
  for (int run = 0; run < runs; ++run) {
    // Open output file in append mode (accumulates data from all runs)
    FILE *fp = fopen("../results/dat/2d_ran_gen_t_100000.dat", "a");
    if (!fp) {
      perror("fopen");
      return EXIT_FAILURE;
    }

    // Reset position to origin for each new run
    pos = (strc){0, 0, 0, 0};

    // Generate fresh, independent seeds for this run
    // Each run gets a unique pair of seeds from the seed generator
    unsigned int seed1 = generate_seed();
    unsigned int seed2 = generate_seed();
    myrand_init(seed1, seed2);

    FILE *ft = NULL;
    if (run == 0)
      ft = fopen("../results/dat/2d_ran_walk_trace.dat", "w");

    /*====================================================================
     * RANDOM WALK LOOP - Execute single random walk trajectory
     *====================================================================*/
    for (pos.step = 0; pos.step < iterations; pos.step++) {
      // Generate random number in [0, 1) to determine step direction
      long double r = myrand();

      // Select direction based on random number (4 equally likely directions)
      // [0, 0.25): move right  (+x direction)
      // [0.25, 0.5): move left  (-x direction)
      // [0.5, 0.75): move up    (+y direction)
      // [0.75, 1): move down    (-y direction)
      if (r < 0.25)
        pos.x += lattice_step;
      else if (r < 0.5)
        pos.x -= lattice_step;
      else if (r < 0.75)
        pos.y += lattice_step;
      else
        pos.y -= lattice_step;

      // Increment time counter (so we can check equality properly since it
      // starts at 0)
      pos.time++;

      // Record position data when target time is reached
      // This allows statistical analysis of position distribution at fixed time
      if (pos.time == t_target) {
        sum_vals_x += pos.x; // Accumulate x-positions for mean calculation
        sum_vals_y += pos.y; // Accumulate x-positions for mean calculation
        // Write: run_number, time, step, x_position
        fprintf(fp, "%d %d %d %ld %ld\n", run, pos.time, pos.step, pos.x,
                pos.y);
      }

      // Optional: Record all steps (currently commented out)
      // Uncomment to save complete trajectory data
      if (ft)
        fprintf(ft, "%d %ld %ld\n", pos.time, pos.x, pos.y);
    }

    if (ft)
      fclose(ft);
    fclose(fp);
    printf("Run %d complete (seeds: %u, %u)\n", run + 1, seed1, seed2);
  }

  /*========================================================================
   * STATISTICAL ANALYSIS - Compute mean and variance of x-positions
   *========================================================================*/

  // Reopen file for reading to compute variance
  FILE *fp = fopen("../results/dat/2d_ran_gen_t_100000.dat", "r");
  if (!fp) {
    perror("fopen");
    return EXIT_FAILURE;
  }

  // Variables for reading data and computing variance
  int dummy_run, dummy_time, dummy_step, status;
  long int x_i, y_i; // x-position from file
  long int idx = 0;  // Counter for number of data points read
  double mean_x = mean_funct(runs, sum_vals_x),
         mean_y = mean_funct(runs, sum_vals_y); // Compute mean
  double sqrdev_x = 0, sqrdev_y = 0; // Sum of squared deviations for variance

  // Read all recorded positions and compute squared deviations
  while ((status = fscanf(fp, "%d %d %d %ld %ld\n", &dummy_run, &dummy_time,
                          &dummy_step, &x_i, &y_i)) == 5) {
    idx++;
    sqrdev_x += (x_i - mean_x) * (x_i - mean_x); // Accumulate (x - mean)²
    sqrdev_y += (y_i - mean_y) * (y_i - mean_y); // Accumulate (y - mean)²
  }

  // Check for file reading errors
  if (status != EOF) {
    perror("fscanf");
    fclose(fp);
    return EXIT_FAILURE;
  }
  fclose(fp);

  // Display statistical results
  printf("MEAN (x position) = %g\n", mean_x);
  printf("MEAN (y position) = %g\n", mean_y);
  printf("x - VAR = %g\n", sqrdev_x / (idx - 1)); // Sample variance x
  printf("y - VAR = %g\n", sqrdev_y / (idx - 1)); // Sample variance y
  printf("idx (processed data points): %ld\n",
         idx); // Number of data points processed

  return EXIT_SUCCESS;
}
