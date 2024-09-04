#!/usr/bin/env gnuplot

set terminal png
set output "./simdata/perf_pcg.png"
set title "cores vs MLUPS/s (PCG)"
set key bottom right

set grid
set xlabel "Num threads"
set ylabel "MLUPS/s"
set yrange [50:500]
set xrange [1:20]

set arrow from graph 0, first 465.91 to graph 1, first 465.91 nohead lt 0 lw 2 lc rgb "red"
set label "465.91 [MLUPS/s]" at graph 0.02, first 475

plot './simdata/pcgperf_20000' title "2000 x 20000" with linespoints, \
'./simdata/pcgperf_400000' title "1000 x 400000" with linespoints, \
'./simdata/pcgperf_2000' title "20000 x 2000" with linespoints
