#!/usr/bin/env gnuplot

set terminal png
set output "perf.png"
set title "cores vs MLUPS/s"

set grid
set xlabel "Num threads"
set ylabel "MLups/s"

plot './simdata/cgperf' title "cgperf" with linespoints, \
    './simdata/pcgperf' title "pcgperf" with linespoints