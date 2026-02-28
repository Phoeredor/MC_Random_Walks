# 🎲 Markov Chains Random Walks

Numerical simulations of **stochastic processes** in C — 1D/2D random walks and lattice gas diffusion — with automated data generation and Gnuplot visualization.

---

## 📁 Project Structure

```
.
├── 01_1d_random_walk/
│   ├── include/
│   │   └── seed_generator.h
│   └── src/
│       ├── main_dat.c     # 1D random walk: trajectories & <x²(t)>
│       └── seed_generator.c
├── 02_2d_random_walk/
│   ├── include/
│   │   └── seed_generator.h
│   └── src/
│       ├── 2d_ran_walk.c  # 2D lattice random walk: trajectories & P(x)
│       └── seed_generator.c
├── 03_diffusion_coefficient/
│   ├── include/
│   │   ├── pcg32.h
│   │   └── seed_generator.h
│   └── src/
│       ├── diff_coef.c    # Lattice gas model: D(ρ,t) measurement
│       ├── pcg32.c
│       └── seed_generator.c
├── generate_data.sh       # Script to compile & run all simulations
├── LICENSE                # MIT License
├── make_plots.gp          # Gnuplot script generating all 8 figures
├── plots/                 # Generated PNG figures folder
└── README.md
```

## 🔬 Simulations

### 1D Random Walk (`01_1d_random_walk`)
- Symmetric random walk on $\mathbb{Z}$ with $\pm 1$ steps
- Ensemble average $\langle x^2(t) \rangle$ over 5000 independent realizations
- Verification of the diffusive scaling $\langle x^2(t) \rangle = t$

### 2D Random Walk (`02_2d_random_walk`)
- Lattice random walk on $\mathbb{Z}^2$ with nearest-neighbor steps
- Trajectory visualization over $10^6$ steps
- Marginal distribution $P(x_1)$ at fixed times $t = 10^3, 10^4, 10^5$ compared with Gaussian fits
- Joint probability $P(x_1, x_2)$ at $t = 10^5$ with theoretical Gaussian surface

### Diffusion Coefficient (`03_diffusion_coefficient`)
- Lattice gas model on a 2D periodic lattice ($L \times L$)
- Measurement of $D(\rho, t) = \langle \Delta r^2 \rangle / (4t)$ with error bars
- Dependence on particle density $\rho$ and lattice size $L$

All simulations use the **PCG32** pseudo-random number generator for high-quality, reproducible randomness.

---

## 📊 Results

### 1D Random Walk

| Trajectory $x(t)$ | $\langle x^2(t) \rangle$ log-log | $\langle x^2(t) \rangle$ mean |
|:---:|:---:|:---:|
| ![1D trajectory](plots/plot1_1d_traj.png) | ![x² log-log](plots/plot2_1d_x2_loglog.png) | ![x² mean](plots/plot3_1d_x2_mean.png) |

### 2D Random Walk

| 2D Trajectory ($10^6$ steps) | Marginal $P(x_1)$ |
|:---:|:---:|
| ![2D trajectory](plots/plot4_2d_traj.png) | ![P(x)](plots/plot5_2d_P_x.png) |

| Joint Probability $P(x_1, x_2)$ at $t = 10^5$ |
|:---:|
| ![P(x1,x2)](plots/plot6_2d_P_x1x2.png) |

### Diffusion Coefficient

| $D(\rho, t)$ — varying density $\rho$ | $D(\rho, t)$ — varying lattice size $L$ |
|:---:|:---:|
| ![D vs rho](plots/plot7_diff_rho.png) | ![D vs L](plots/plot8_diff_L.png) |

---

## 🔧 Build & Run

**Prerequisites:** GCC, Gnuplot

```bash
# Generate all data and plots in one step
bash generate_data.sh
gnuplot make_plots.gp
```

Or compile individual simulations:
```bash
cd 01_1d_random_walk
gcc -O3 src/main_dat.c src/seed_generator.c -o program_dat -Iinclude -lm
```

---

## 📖 References

- **Barone, L. M., Marinari, E., Organtini, G., Ricci-Tersenghi, F.**
  *Scientific Programming: C-Language, Algorithms and Models in Science.*
  World Scientific, 2013. ISBN: 978-981-4513-40-1.
