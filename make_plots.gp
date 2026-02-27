set terminal pngcairo size 1400,1155 enhanced font 'Arial,24' rounded

# Modern Aesthetic Options
set border lw 1.5
set tics scale 0.75
set grid back lc rgb '#e5e5e5' dt 1 lw 1
set lmargin 12
set rmargin 5
set tmargin 3
set bmargin 5
set style line 1 lc rgb '#1f77b4' pt 7 ps 0.6 lw 2 # Blue
set style line 2 lc rgb '#ff7f0e' pt 5 ps 0.6 lw 2 # Orange
set style line 3 lc rgb '#2ca02c' pt 9 ps 0.6 lw 2 # Green
set style line 4 lc rgb '#d62728' pt 7 ps 0.6 lw 2 # Red
set style line 5 lc rgb '#9467bd' pt 7 ps 0.6 lw 2 # Purple
# Plot 1: 1D RW Trajectory
set output 'plots/plot1_1d_traj.png'
set title "1D Random Walk Trajectory"
set xlabel "Time t"
set ylabel "Position x(t)"
unset logscale
set xrange [0:100000]
set autoscale y
plot "01_1d_random_walk/results/dat/ran_gen_1traj.dat" using 4:2 with lines ls 1 title "x(t)"

# Plot 2: 1D RW x^2 log-log
set output 'plots/plot2_1d_x2_loglog.png'
set title "1D Random Walk x^2(t) [Single Trajectory]"
set autoscale xy
set xlabel "Time t"
set ylabel "x^2(t)"
set logscale xy
plot "01_1d_random_walk/results/dat/ran_gen_1traj.dat" using 4:3 with lines ls 4 title "x^2(t)", \
     x with lines title "Slope 1" lc rgb "#333333" dashtype 2

# Plot 3: 1D RW Mean <x^2> log-log
set output 'plots/plot3_1d_x2_mean.png'
set title "1D Random Walk {/Symbol \341}x^2(t){/Symbol \361} [Mean over 5000 instances]"
set xlabel "Time t"
set ylabel "{/Symbol \341}x^2(t){/Symbol \361}"
set logscale xy
plot "01_1d_random_walk/results/dat/x2_mean_5000.dat" using 1:2 title "{/Symbol \341}x^2(t){/Symbol \361}" with points ls 3 ps 1.5, \
     x with lines title "Slope 1" lc rgb "#333333" dashtype 2 lw 3

# Plot 4: 2D RW Trajectory (1M steps)
set terminal pngcairo size 1400,1155 enhanced font 'Arial,24' rounded
set output 'plots/plot4_2d_traj.png'
set title "2D Random Walk Trajectory (1,000,000 steps)"
set xlabel "x_1(t)"
set ylabel "x_2(t)"
unset logscale
set size ratio -1
set xtics rotate by -45
plot "02_2d_random_walk/results/dat/traj_1M.dat" using 2:3 with lines ls 5 title "Trajectory"

# Plot 5: 2D RW P(x) histogram with gaussian fit
set terminal pngcairo size 1400,1155 enhanced font 'Arial,24' rounded
set size ratio 0
set xtics norotate
set output 'plots/plot5_2d_P_x.png'
set title "Probability Distribution P(x_1) in 2D RW"
set xlabel "x_1"
set ylabel "P(x_1(t))"
unset logscale
set xrange [-800:800]
bin_width(x, s) = s*floor(x/s)
N = 10000.0

# Define Gaussians for 2D walk x_1 variance is t/2.
# But lattice walks have probability on either even or odd sites, so density is scaled.
# We'll just plot them normalized nicely by bin width.
# width depends on t to look nice:
w1000 = 8.0
w10000 = 25.0
w100000 = 80.0

P(x, t) = (1.0/sqrt(pi*t)) * exp(-(x**2)/t)

plot \
    "02_2d_random_walk/results/dat/res_100000.dat" using (bin_width($4, w100000)):(1.0/(N*w100000)) smooth freq with points ls 1 ps 1.5 title "t=10^5", \
    P(x, 100000.0) with lines lw 3.0 dt 1 lc rgb "#333333" notitle, \
    "02_2d_random_walk/results/dat/res_10000.dat" using (bin_width($4, w10000)):(1.0/(N*w10000)) smooth freq with points ls 3 ps 1.5 title "t=10^4", \
    P(x, 10000.0) with lines lw 3.0 dt 4 lc rgb "#333333" notitle, \
    "02_2d_random_walk/results/dat/res_1000.dat" using (bin_width($4, w1000)):(1.0/(N*w1000)) smooth freq with points ls 4 ps 1.5 title "t=10^3", \
    P(x, 1000.0) with lines lw 3.0 dt 3 lc rgb "#333333" notitle

# Plot 6: 2D RW P(x1, x2) 3D plot
set terminal pngcairo size 1400,1000 enhanced font 'Arial,24' rounded
set bmargin 7
set tmargin 1
set output 'plots/plot6_2d_P_x1x2.png'
set title "P(x_1(t), x_2(t)) at t=10^5"
set key right top Right samplen 2
set xlabel "x_1(t)"
set ylabel "x_2(t)"
set xrange [-800:800]
set yrange [-800:800]
set zlabel "P(x_1, x_2)" rotate by 90 offset -4, 0
unset dgrid3d
set isosamples 30,30
set hidden3d
set view 55, 30, 1.15, 1.15
set xyplane relative 0.375
# Theoretical surface:
G(x,y) = (1.0/(pi*100000.0)) * exp(-(x**2 + y**2)/100000.0)
splot \
    G(x,y) with lines lc rgb "#333333" title "Theoretical Gaussian Limit", \
    "02_2d_random_walk/results/dat/res_100000.dat" every 2 using 4:5:(G($4,$5)) with impulses lw 1 lc rgb "#B121EF" notitle, \
    "" every 2 using 4:5:(G($4,$5)) with points pt 7 ps 1.2 lc rgb "#B121EF" title "Numerical Data", \
    "" every 2 using 4:5:(G($4,$5)) with points pt 6 ps 1.2 lw 0.5 lc rgb "black" notitle

# Plot 7: Diffusion Coefficient (rho variation)
set terminal pngcairo size 1400,1155 enhanced font 'Arial,24' rounded
set bmargin 5
set tmargin 3
set output 'plots/plot7_diff_rho.png'
set title "Diffusion Coefficient D({/Symbol r}, t) for L=80"
set xlabel "Time (sweeps)"
set ylabel "D({/Symbol r}, t)"
unset dgrid3d
unset logscale x
set xrange [0:2000]
set yrange [0:0.25]
set xtics 0, 500, 2000
set format x "%g"
plot \
    "03_diffusion_coefficient/results/out_rho0.1_L80.dat" using 1:3:5 with yerrorbars ls 1 pt 7 ps 1.5 title "rho=0.1", \
    "03_diffusion_coefficient/results/out_rho0.2_L80.dat" using 1:3:5 with yerrorbars ls 2 pt 5 ps 1.5 title "rho=0.2", \
    "03_diffusion_coefficient/results/out_rho0.3_L80.dat" using 1:3:5 with yerrorbars ls 3 pt 9 ps 1.5 title "rho=0.3", \
    "03_diffusion_coefficient/results/out_rho0.4_L80.dat" using 1:3:5 with yerrorbars ls 4 pt 7 ps 1.5 title "rho=0.4", \
    "03_diffusion_coefficient/results/out_rho0.5_L80.dat" using 1:3:5 with yerrorbars ls 5 pt 5 ps 1.5 title "rho=0.5", \
    "03_diffusion_coefficient/results/out_rho0.6_L80.dat" using 1:3:5 with yerrorbars lc rgb "#8c564b" pt 9 ps 1.5 title "rho=0.6", \
    "03_diffusion_coefficient/results/out_rho0.7_L80.dat" using 1:3:5 with yerrorbars lc rgb "#e377c2" pt 7 ps 1.5 title "rho=0.7", \
    "03_diffusion_coefficient/results/out_rho0.8_L80.dat" using 1:3:5 with yerrorbars lc rgb "#7f7f7f" pt 5 ps 1.5 title "rho=0.8", \
    "03_diffusion_coefficient/results/out_rho0.9_L80.dat" using 1:3:5 with yerrorbars lc rgb "#bcbd22" pt 9 ps 1.5 title "rho=0.9"

# Plot 8: Diffusion Coefficient (L variation)
set output 'plots/plot8_diff_L.png'
set title "Diffusion Coefficient D({/Symbol r}=0.6, t) vs L"
set xlabel "Time (sweeps)"
set ylabel "D({/Symbol r}, t)"
unset logscale x
set xrange [0:2000]
set yrange [0.060:0.075]
set xtics 0, 500, 2000
set ytics 0.060, 0.004, 0.075
plot \
    "03_diffusion_coefficient/results/out_rho0.6_L20.dat" using 1:3:5 with yerrorbars ls 2 ps 1.5 title "L=20", \
    "03_diffusion_coefficient/results/out_rho0.6_L40.dat" using 1:3:5 with yerrorbars ls 3 ps 1.5 title "L=40", \
    "03_diffusion_coefficient/results/out_rho0.6_L80.dat" using 1:3:5 with yerrorbars ls 4 ps 1.5 title "L=80"

