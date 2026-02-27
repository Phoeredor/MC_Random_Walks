// 1D Random Walk Generator using PCG32 PRNG
// Build: gcc main_dat.c seed_generator.c -o program_dat -lm

#include "../include/seed_generator.h"
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // for getpid()

//=======================================================
//  UTILITY FUNCTIONS
//=======================================================

// Error handling for memory allocation
int handle_mem_err() {
  printf("ERROR! MEMORY NOT AVAILABLE!\n");
  return EXIT_FAILURE;
}
// Allocate a 1D array of doubles
int mtrx_alloc(double **mtrx, size_t dim) {
  *mtrx = malloc(dim * sizeof(**mtrx)); // p to p matrix allocation
  if (*mtrx == NULL)                    // memory check
    return handle_mem_err();

  return EXIT_SUCCESS;
}

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

uint32_t pcg32_random_r(pcg32_random_t *rng) {
  uint64_t oldstate = rng->state;
  // Advance internal state
  rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
  // Calculate output function (XSH RR), uses old state for max ILP
  uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
  uint32_t rot = (uint32_t)(oldstate >> 59u);
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void pcg32_srandom_r(pcg32_random_t *rng, uint64_t initstate,
                     uint64_t initseq) {
  rng->state = 0U;
  rng->inc = (initseq << 1u) | 1u;
  pcg32_random_r(rng);
  rng->state += initstate;
  pcg32_random_r(rng);
}

pcg32_random_t pcg32_random_state;

//=======================================================
//  INITIALIZATION
//=======================================================
void myrand_init(unsigned long int initstate, unsigned long int initseq) {
  pcg32_srandom_r(&pcg32_random_state, (uint64_t)initstate, (uint64_t)initseq);
}

double myrand(void) { // generate uniform random number in [0,1)
  return (double)pcg32_random_r(&pcg32_random_state) /
         ((double)UINT32_MAX + 1.0);
}

//=======================================================
//  MAIN FUNCTION
//=======================================================
int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stdout, ">>>> PROGRAM INSTRUCTIONS <<<<\n");
    fprintf(
        stderr,
        "Compile with: %s <number of runs> <number of iterations per run'>\n",
        argv[0]);

    return EXIT_FAILURE;
  }

  seedgen_init(12345ULL, 67890ULL); // seed gen initialization
  int runs = 0, iterations = 0;

  runs = atof(argv[1]);       // total number of runs
  iterations = atof(argv[2]); // iterations for single run

  double *A = NULL; // array of random generated values
  for (int run = 0; run < runs; ++run) {
    unsigned int seed1 = generate_seed();
    unsigned int seed2 = generate_seed();
    myrand_init(seed1, seed2); // rand number generation

    int position = 0, time = 0;                     // initial conditions
    if (mtrx_alloc(&A, iterations) != EXIT_SUCCESS) // matrix allocation
      return EXIT_FAILURE;

    // open output file in append mode
    FILE *fp = fopen("../results/dat/ran_gen.dat", "a");
    if (!fp) {
      perror("fopen");
      free(A);
      return EXIT_FAILURE;
    }

    for (int i = 0; i < iterations; i++) {
      A[i] = myrand();
      // random walk step
      if (A[i] > 0.5)
        position += 1;
      else
        position -= 1;

      int pos_sqr = position * position; // x^2
      fprintf(fp, "%d %d %d %d\n", i, position, pos_sqr, time);
      time++;
    }
    // close file and free memory
    fclose(fp);
    free(A);
    printf("Run %d complete (seeds: %u, %u)\n", run + 1, seed1, seed2);
  }

  // compute ensemble mean <x^2(t)>

  // initialize accumulator array
  double *sum = calloc(iterations, sizeof(double));
  if (!sum) {
    fprintf(stderr, "Memory allocation failed.\n");
    return EXIT_FAILURE;
  }

  // read all runs from data file
  FILE *fp = fopen("../results/dat/ran_gen.dat", "r");
  if (!fp) {
    perror("fopen");
    free(sum);
    return EXIT_FAILURE;
  }
  int dummy_i, dummy_pos, pos_sqr, dummy_time; // dummy variables
  int status;                                  // used for flowing inside file
  int run_idx = 0, t_idx = 0;                  // run index - time index
  while ((status = fscanf(fp, "%d %d %d %d", &dummy_i, &dummy_pos, &pos_sqr,
                          &dummy_time)) == 4) {
    sum[t_idx] += pos_sqr;   // sum upgrading
    t_idx++;                 // time increasing
    if (t_idx == iterations) // go to next run
    {
      t_idx = 0;
      run_idx++;
      if (run_idx == runs)
        break; // end file -> read completed
    }
  }
  if (status != EOF && status != 4 &&
      run_idx != runs) // reading file error check
  {
    perror("fscanf");
    fclose(fp);
    return EXIT_FAILURE;
  }
  fclose(fp);

  // write <x^2(t)> values to file
  fp = fopen("../results/dat/x2_mean.dat", "w");
  if (!fp) {
    perror("fopen");
    free(sum);
    return EXIT_FAILURE;
  }
  for (int t = 0; t < iterations; t++) {
    double avg = sum[t] / runs;
    fprintf(fp, "%d %f\n", t, avg); // time & <x^2>
  }

  fclose(fp);
  free(sum);

  printf("Mean <x^2> written to '../results/dat/x2_mean.dat'\n");

  return EXIT_SUCCESS;
}
