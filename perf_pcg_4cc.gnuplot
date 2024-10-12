#!/usr/bin/env gnuplot

set terminal png
set output "./simdata4ccnuma/perf_pcg.png"
set title "cores vs MLUPS/s (PCG)"
set key bottom right

set grid
set xlabel "Num threads"
set ylabel "MLUPS/s"
set yrange [0:2800]
set xrange [1:75]

plot './simdata4ccnuma/pcgperf_20000' title "2000 x 20000" with linespoints, \
'./simdata4ccnuma/pcgperf_400000' title "1000 x 400000" with linespoints, \
'./simdata4ccnuma/pcgperf_2000' title "20000 x 2000" with linespoints