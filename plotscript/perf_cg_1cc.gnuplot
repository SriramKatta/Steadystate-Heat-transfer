#!/usr/bin/env gnuplot

set terminal png
set output "./simdata1ccnuma/perf_cg.png"
set title "cores vs MLUPS/s (CG)"
set key bottom right

set grid
set xlabel "Num threads"
set ylabel "MLUPS/s"
set yrange [0:1100]
set xrange [0:20]

set arrow from graph 0, first 931.8 to graph 1, first 931.8 nohead lt 0 lw 2 lc rgb "red"
set label "931.8 [MLUPS/s] no WA" at graph 0.02, first 945

set arrow from graph 0, first 854 to graph 1, first 854nohead lt 0 lw 2 lc rgb "blue"
set label "854 [MLUPS/s] with WA" at graph 0.02, first 825

plot './simdata1ccnuma/cgperf_20000' title "2000 x 20000" with linespoints, \
'./simdata1ccnuma/cgperf_400000' title "1000 x 400000" with linespoints, \
'./simdata1ccnuma/cgperf_2000' title "20000 x 2000" with linespoints