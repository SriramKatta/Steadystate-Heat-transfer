#!/usr/bin/env gnuplot

set terminal png
set output "./simdata/perf_cg.png"
set title "cores vs MLUPS/s (CG)"
set key bottom right

set grid
set xlabel "Num threads"
set ylabel "MLUPS/s"
set yrange [200:800]
set xrange [1:20]

set arrow from graph 0, first 732.14 to graph 1, first 732.14 nohead lt 0 lw 2 lc rgb "red"
set label "732.14 [MLUPS/s]" at graph 0.02, first 744

plot './simdata/cgperf_20000' title "2000 x 20000" with linespoints, \
'./simdata/cgperf_400000' title "1000 x 400000" with linespoints, \
'./simdata/cgperf_2000' title "20000 x 2000" with linespoints