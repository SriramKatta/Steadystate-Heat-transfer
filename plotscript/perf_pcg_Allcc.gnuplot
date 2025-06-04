#!/usr/bin/env gnuplot

set terminal png
set output "./simdataAllccnuma/perf_pcg.png"
set title "cores vs MLUPS/s (PCG)"
set key top left

set grid
set xlabel "Num threads"
set ylabel "MLUPS/s"
set yrange [0:3500]
set xrange [0:75]

plot './simdataAllccnuma/pcgperf_20000' title "2000 x 20000" with linespoints, \
'./simdataAllccnuma/pcgperf_400000' title "1000 x 400000" with linespoints, \
'./simdataAllccnuma/pcgperf_2000' title "20000 x 2000" with linespoints