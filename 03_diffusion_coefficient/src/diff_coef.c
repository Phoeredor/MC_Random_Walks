/* Lattice Gas Diffusion Coefficient Simulation
 * Usage: ./program L rho num_sweeps meas_per_sweep num_samples output.dat
 */
#include "../include/pcg32.h"
#include "../include/seed_generator.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM 2 // lattice system dimension
#define STRING_LENGTH 128
#define MY_EMPTY (-1L)
// #define MY_DEBUG // DEBUGGING ->  enable heavy internal checks or "gcc
// -DMY_DEBUG"

// debug function prototype
static void debug_init_lattice(long int trueN);

//=======================================================
//  GLOBAL VARIABLES
//=======================================================
static long int L, VOLUME;
static double rho;
static long int num_sweeps, num_measurements, measurement_period, num_samples,
    meas_per_sweep;
static char datafile[STRING_LENGTH];

/* 2D lattice flattened to 1D */
static long int *particleOfSite; // dimension: VOLUME = L*L
#define SITE(x, y) particleOfSite[(x) * L + (y)]

/* particle positions: [particle][dim] flattened to 1D */
static long int *positionOfParticle;
#define POS(p, mu) positionOfParticle[(p) * DIM + (mu)]

static long int *zeroPositionOfParticle;
#define ZERO_POS(p, mu) zeroPositionOfParticle[(p) * DIM + (mu)]

static long int *truePositionOfParticle;
#define TRUE_POS(p, mu) truePositionOfParticle[(p) * DIM + (mu)]

/* neighbours for PBC */
static long int *plusNeighbor;
static long int *minusNeighbor;
/* measurements */
static double *averageDeltaR2;
static double *errorDeltaR2;

//=======================================================
//  UTILITY FUNCTIONS
//=======================================================

// Error handling
static void handleErrAll(const char *matrix_name, size_t size) {
  fprintf(stderr,
          "ERROR: Matrix Memory allocation failed for '%s' (%zu bytes)\n",
          matrix_name, size);
  exit(EXIT_FAILURE);
}
// 2D matrix allocation
long int *mtrxAlloc2d(long int rows, long int cols, const char *name) {
  size_t bytes = rows * cols * sizeof(long int); // SIZE REQUESTED
  long int *mtrx = malloc(bytes);                // MATRIX ALLOCATION
  if (mtrx == NULL)                              // MEMORY CHECK
    handleErrAll(name, bytes);
  return mtrx;
}

// 1D array allocation (long int)
static long int *mtrxLongIntAlloc(long int n, const char *name) {
  size_t bytes = (size_t)n * sizeof(long int); // SIZE REQUESTED
  long int *m = malloc(bytes);                 // MATRIX ALLOCATION
  if (!m)                                      // CHECK MEMORY
    handleErrAll(name, bytes);

  return m;
}
// 1D array allocation (double)
static double *mtrxDoubleAlloc(long int n, const char *name) {
  size_t bytes = (size_t)n * sizeof(double);
  double *m = malloc(bytes);
  if (!m)
    handleErrAll(name, bytes);
  return m;
}

//=======================================================
//  INITIALIZATION
//=======================================================
void myInit(void) {
  // matrices allocation
  particleOfSite = mtrxAlloc2d(L, L, "particleOfSite");
  positionOfParticle = mtrxAlloc2d(VOLUME, DIM, "positionOfParticle");
  zeroPositionOfParticle = mtrxAlloc2d(VOLUME, DIM, "zeroPositionOfParticle");
  truePositionOfParticle = mtrxAlloc2d(VOLUME, DIM, "truePositionOfParticle");

  plusNeighbor = mtrxLongIntAlloc(L, "plusNeighbor");
  minusNeighbor = mtrxLongIntAlloc(L, "minusNeighbor");

  for (long int i = 0; i < L; i++) {
    plusNeighbor[i] = i + 1;
    minusNeighbor[i] = i - 1;
  }
  plusNeighbor[L - 1] = 0;
  minusNeighbor[0] = L - 1;

  if ((num_measurements * measurement_period) != num_sweeps) {
    printf("ERROR: number of steps not a multiple number of measurements\n");
    exit(EXIT_FAILURE);
  }

  averageDeltaR2 = mtrxDoubleAlloc(num_measurements, "averageDeltaR2");
  errorDeltaR2 = mtrxDoubleAlloc(num_measurements, "errorDeltaR2");
  /* zero measurement arrays */
  for (long int m = 0; m < num_measurements; m++) {
    averageDeltaR2[m] = 0.0;
    errorDeltaR2[m] = 0.0;
  }
}

// Lattice initialization: place particles randomly with density rho
static long int initLattice(double rho) {
  long int trueN = 0;

  /* empty lattice */
  for (long int x = 0; x < L; ++x)
    for (long int y = 0; y < L; ++y)
      SITE(x, y) = MY_EMPTY;

  // Filling lattice with particles
  for (int x = 0; x < L; x++) {
    for (int y = 0; y < L; y++) {
      // place particle with probability rho
      long double r = myrand();
      if (r < rho) {
        long int p = trueN;
        SITE(x, y) = p;
        POS(p, 0) = x;
        POS(p, 1) = y;
        ZERO_POS(p, 0) = x;
        ZERO_POS(p, 1) = y;
        TRUE_POS(p, 0) = x;
        TRUE_POS(p, 1) = y;
        trueN++;
      }
    }
  }
#ifdef MY_DEBUG
  debug_init_lattice(trueN);
#endif

  return trueN;
}

void updateLattice(long int trueN) {
  // 1 sweep = trueN update try
  for (long int attempt = 0; attempt < trueN; ++attempt) {
    // 1. pick random particle in [0, trueN-1]
    long int p = (long int)(myrand() * (double)trueN);

#ifdef MY_DEBUG
    if (p < 0 || p >= trueN) {
      fprintf(
          stderr,
          ">>>> DEBUG ERROR: invalid particle index p=%ld in updateLattice\n",
          p);
      exit(EXIT_FAILURE);
    }
#endif

    // 2. read actual position on lattice
    long int x = POS(p, 0);
    long int y = POS(p, 1);

    // 3. random direction
    int dir = (int)(4.0 * myrand()); // 0,1,2,3

    // error direction check
    if (dir < 0 || dir > 3) {
      fprintf(stderr, ">>>> DEBUG ERROR: invalid dir=%d in updateLattice\n",
              dir);
      exit(EXIT_FAILURE);
    }

    // 4. neighbor calculation
    long int nx = x;
    long int ny = y;

    switch (dir) {
    case 0:
      nx = plusNeighbor[x]; //+x
      break;
    case 1:
      nx = minusNeighbor[x]; //-x
      break;
    case 2:
      ny = plusNeighbor[y]; //+y
      break;
    case 3:
      ny = minusNeighbor[y]; //-y
      break;
    }

    // 5. occupied site -> FAILED TRANSFER
    if (SITE(nx, ny) != MY_EMPTY) {
      continue;
    }

    // 6. free site -> particle from (x,y) to (nx,ny)
    SITE(nx, ny) = p;
    SITE(x, y) = MY_EMPTY;

    POS(p, 0) = nx;
    POS(p, 1) = ny;

    // update unwrapped (absolute) position
    switch (dir) {
    case 0:
      TRUE_POS(p, 0)++;
      break;
    case 1:
      TRUE_POS(p, 0)--;
      break;
    case 2:
      TRUE_POS(p, 1)++;
      break;
    case 3:
      TRUE_POS(p, 1)--;
      break;
    }
  }

#ifdef MY_DEBUG
  // Check number of particles
  long int count = 0;
  for (long int x = 0; x < L; x++) {
    for (long int y = 0; y < L; y++) {
      if (SITE(x, y) != MY_EMPTY) {
        count++;
      }
    }
  }
  if (count != trueN) {
    fprintf(stderr,
            ">>>> DEBUG ERROR: particle number changed in updateLattice "
            "(count=%ld, trueN=%ld)\n",
            count, trueN);
    exit(EXIT_FAILURE);
  }
#endif
}

// Compute mean square displacement <Delta r^2> over all particles

double measure(long int trueN) {
#ifdef MY_DEBUG
  if (trueN <= 0) {
    fprintf(stderr, ">>>> DEBUG ERROR: trueN <= 0 in measure\n");
    exit(EXIT_FAILURE);
  }
#endif

  double sqrDist = 0.0, meanSqrShift;
  for (long int p = 0; p < trueN; ++p) {
    for (int mu = 0; mu < DIM; ++mu) {
      double dl = (double)(TRUE_POS(p, mu) - ZERO_POS(p, mu));
      sqrDist += dl * dl;
    }
  }
  meanSqrShift = sqrDist / (double)trueN;
  return meanSqrShift;
}

void myEnd(FILE *fp) {
  free(particleOfSite);
  free(positionOfParticle);
  free(zeroPositionOfParticle);
  free(truePositionOfParticle);
  free(plusNeighbor);
  free(minusNeighbor);
  free(averageDeltaR2);
  free(errorDeltaR2);
  fclose(fp);
}

//=======================================================
//  MAIN FUNCTION
//=======================================================

int main(int argc, char **argv) {
  if (argc != 7) {
    fprintf(stdout, "---- PROGRAM INSTRUCTIONS ----\n");
    fprintf(stderr,
            "Compile with: %s L rho num_sweeps meas_per_sweep num_samples "
            "datafile\n",
            argv[0]);
    fprintf(stdout, "L = lattice size\n");
    fprintf(
        stdout,
        "rho = probability to have a particle in a site, must be in (0,1)\n");
    fprintf(stdout, "num_sweeps = normalized clocks: 1 sweep is 1 unit time\n");
    fprintf(stdout,
            "meas_per_sweep = number of measurements done for single sweep\n");
    fprintf(stdout, "num_samples = \n");

    return EXIT_FAILURE;
  }
  L = strtol(argv[1], NULL, 10);
  rho = atof(argv[2]);
  num_sweeps = strtol(argv[3], NULL, 10);
  meas_per_sweep = strtol(argv[4], NULL, 10);
  num_samples = strtol(argv[5], NULL, 10);
  strncpy(datafile, argv[6], STRING_LENGTH - 1);
  datafile[STRING_LENGTH - 1] = '\0'; // ensure null-termination

  VOLUME = L * L;
  num_measurements = 100;
  measurement_period = num_sweeps / num_measurements;

  // random seed initialization: one global seeding
  seedgen_init(12345ULL, 67890ULL);
  unsigned int seed1 = generate_seed();
  unsigned int seed2 = generate_seed();
  // random pcg initialization
  myrand_init(seed1, seed2);

  myInit();
  long int sweep = 0;
  FILE *fp = fopen(datafile, "w");
  if (!fp) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  for (long int sample = 0; sample < num_samples; sample++) {
    long int trueN = initLattice(rho);

    for (sweep = 1; sweep <= num_sweeps; sweep++) {
      updateLattice(trueN);

      if (sweep > 0 && sweep % measurement_period == 0) {
        long m = sweep / measurement_period - 1; // index 0...num_meas -1
        double deltaR2 = measure(trueN);

        averageDeltaR2[m] += deltaR2;
        errorDeltaR2[m] += deltaR2 * deltaR2; // for the variance
      }
    }
  }
  fprintf(
      fp,
      "# L = %ld  rho_input = %.3f  num_sweeps = %ld    num_samples = %ld\n", L,
      rho, num_sweeps, num_samples);
  fprintf(fp, "# sweep   deltaR2_mean      D_t_mean        err_deltaR2\n");

  // normalize averages and compute errors
  for (long m = 0; m < num_measurements; ++m) {
    double mean = averageDeltaR2[m] / (double)num_samples;
    double mean2 = errorDeltaR2[m] / (double)num_samples;
    double var = mean2 - mean * mean;
    double err = (var > 0.0) ? sqrt(var / (double)num_samples) : 0.0;

    // t = (m+1) * measurement_period
    long sweep = (m + 1) * measurement_period;
    double D_t = mean / (4.0 * (double)sweep);
    double err_D = err / (4.0 * (double)sweep);

    fprintf(fp, "%ld %.12f %.12f %.12f %.12f\n", sweep, mean, D_t, err, err_D);
  }

  myEnd(fp);

  return EXIT_SUCCESS;
}

//=======================================================
//  DEBUG CONTROLS
//=======================================================

#ifdef MY_DEBUG
static void debug_init_lattice(long int trueN) {
  // 1) trueN value check
  if (trueN > VOLUME) {
    fprintf(stderr, ">>>> DEBUG ERROR: trueN > VOLUME in initLattice\n");
    exit(EXIT_FAILURE);
  }

  // 2) every sites has MY_EMPTY or index [0,trueN)
  long count_sites = 0;
  for (long x = 0; x < L; x++) {
    for (long y = 0; y < L; y++) {
      long p = SITE(x, y);
      if (p == MY_EMPTY)
        continue;
      if (p < 0 || p >= trueN) {
        fprintf(
            stderr,
            ">>>> DEBUG ERROR: invalid particle index %ld at site (%ld,%ld)\n",
            p, x, y);
        exit(EXIT_FAILURE);
      }
      count_sites++;
    }
  }

  if (count_sites != trueN) {
    fprintf(
        stderr,
        ">>>> DEBUG ERROR: mismatch count_sites=%ld trueN=%ld in initLattice\n",
        count_sites, trueN);
    exit(EXIT_FAILURE);
  }

  // 3) POS/ZERO_POS/TRUE_POS and one particle single-site occupation check
  long *seen = calloc((size_t)trueN, sizeof(long));
  if (!seen) {
    fprintf(stderr, ">>>> DEBUG ERROR: calloc failed in initLattice\n");
    exit(EXIT_FAILURE);
  }

  for (long x = 0; x < L; x++) {
    for (long y = 0; y < L; y++) {
      long p = SITE(x, y);
      if (p == MY_EMPTY)
        continue;

      seen[p]++;

      if (POS(p, 0) != x || POS(p, 1) != y) {
        fprintf(stderr,
                ">>>> DEBUG ERROR: POS mismatch for p=%ld at site (%ld,%ld)\n",
                p, x, y);
        exit(EXIT_FAILURE);
      }
      if (ZERO_POS(p, 0) != x || ZERO_POS(p, 1) != y) {
        fprintf(stderr, ">>>> DEBUG ERROR: ZERO_POS mismatch for p=%ld\n", p);
        exit(EXIT_FAILURE);
      }
      if (TRUE_POS(p, 0) != x || TRUE_POS(p, 1) != y) {
        fprintf(stderr, ">>>> DEBUG ERROR: TRUE_POS mismatch for p=%ld\n", p);
        exit(EXIT_FAILURE);
      }
    }
  }

  for (long p = 0; p < trueN; p++) {
    if (seen[p] != 1) {
      fprintf(
          stderr,
          ">>>> DEBUG ERROR: particle %ld occupies %ld sites (should be 1)\n",
          p, seen[p]);
      exit(EXIT_FAILURE);
    }
  }
  free(seen);
}
#endif
