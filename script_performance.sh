#!/bin/bash -l

module load intel likwid

make clean

likwid=off CXX=icpx make > /dev/null

srun --cpu-freq=2000000-2000000 likwid-pin -q -c M0:0-17 ./perf 5000 5000

make clean
