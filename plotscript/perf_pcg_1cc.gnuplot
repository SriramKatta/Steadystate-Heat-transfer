#!/usr/bin/env gnuplot

set terminal png
set output "./simdata1ccnuma/perf_pcg.png"
set title "cores vs MLUPS/s (PCG)"
set key top left

set grid
set xlabel "Num threads"
set ylabel "MLUPS/s"
set yrange [0:1100]
set xrange [0:20]

set arrow from graph 0, first 602.94 to graph 1, first 602.94 nohead lt 0 lw 2 lc rgb "red"
set label "602.94 [MLUPS/s] no WA" at graph 0.02, first 630

set arrow from graph 0, first 569.44 to graph 1, first 569.44 nohead lt 0 lw 2 lc rgb "blue"
set label "569.44 [MLUPS/s] with WA" at graph 0.02, first 535

plot './simdata1ccnuma/pcgperf_400000' title "1000 x 400000" with linespoints, \
'./simdata1ccnuma/pcgperf_20000' title "2000 x 20000" with linespoints, \
'./simdata1ccnuma/pcgperf_2000' title "20000 x 2000" with linespoints
