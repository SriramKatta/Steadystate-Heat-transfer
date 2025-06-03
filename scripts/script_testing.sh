#!/bin/bash -l

module load intel likwid

make clean

LIKWID=on CXX=icpx make -j > /dev/null

./test

make clean
make cleanexe