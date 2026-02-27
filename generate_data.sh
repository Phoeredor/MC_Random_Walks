#!/bin/bash
set -e

BASE="$(cd "$(dirname "$0")" && pwd)"
cd "$BASE"

echo "=== Compiling Programs ==="
cd "$BASE/01_1d_random_walk"
gcc -O3 src/main_dat.c src/seed_generator.c -o program_dat -Iinclude -lm
cd "$BASE/02_2d_random_walk"
gcc -O3 src/2d_ran_walk.c src/seed_generator.c -o program_2d -Iinclude -lm
cd "$BASE/03_diffusion_coefficient"
gcc -O3 src/diff_coef.c src/seed_generator.c src/pcg32.c -o program_diff -Iinclude -lm

mkdir -p "$BASE/plots"

echo "=== Generating Data for 1D Random Walk ==="
cd "$BASE/01_1d_random_walk"
mkdir -p results/dat
cd src
rm -f ../results/dat/*.dat
# Plot 1 & 2
../program_dat 1 100000
cp ../results/dat/ran_gen.dat ../results/dat/ran_gen_1traj.dat
rm -f ../results/dat/ran_gen.dat ../results/dat/x2_mean.dat
# Plot 3
../program_dat 5000 1000
cp ../results/dat/x2_mean.dat ../results/dat/x2_mean_5000.dat
cd ..
cd ..

echo "=== Generating Data for 2D Random Walk ==="
cd "$BASE/02_2d_random_walk"
mkdir -p results/dat
cd src
rm -f ../results/dat/*.dat
# Plot 4
echo "1 1000000 1000000" | ../program_2d
cp ../results/dat/2d_ran_walk_trace.dat ../results/dat/traj_1M.dat

# Plot 5 & 6
rm -f ../results/dat/2d_ran_gen_t_100000.dat
echo "10000 1000 1000" | ../program_2d
mv ../results/dat/2d_ran_gen_t_100000.dat ../results/dat/res_1000.dat

rm -f ../results/dat/2d_ran_gen_t_100000.dat
echo "10000 10000 10000" | ../program_2d
mv ../results/dat/2d_ran_gen_t_100000.dat ../results/dat/res_10000.dat

rm -f ../results/dat/2d_ran_gen_t_100000.dat
echo "10000 100000 100000" | ../program_2d
mv ../results/dat/2d_ran_gen_t_100000.dat ../results/dat/res_100000.dat
cd ..
cd ..

echo "=== Generating Data for Diffusion Coefficient ==="
cd "$BASE/03_diffusion_coefficient"
mkdir -p results
cd src
# Plot 7 (L=80, rho=0.1..0.9)
for rho in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
    echo "Running diff L=80 rho=$rho"
    ../program_diff 80 $rho 2000 100 50 ../results/out_rho${rho}_L80.dat
done

# Plot 8 (rho=0.6, L=20,40,80)
for L in 20 40 80; do
    echo "Running diff L=$L rho=0.6"
    ../program_diff $L 0.6 2000 100 50 ../results/out_rho0.6_L${L}.dat
done
cd ..
cd ..

echo "Data Generation Complete!"
