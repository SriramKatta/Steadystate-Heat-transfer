#!/usr/bin/env gnuplot

set terminal png
set output "./simdata/perf_cg.png"
set title "cores vs MLUPS/s (CG)"

set grid
set xlabel "Num threads"
set ylabel "MLUPS/s"

plot './simdata/cgperf_20000' title "2000 x 20000" with linespoints, \
'./simdata/cgperf_2000' title "20000 x 2000" with linespoints, \
'./simdata/cgperf_400000' title "1000 x 400000" with linespoints