#!/bin/bash -l

for ver in cg pcg; do
    for numa in 1 All; do
        gnuplot ./plotscript/perf_${ver}_${numa}cc.gnuplot 
    done
done
