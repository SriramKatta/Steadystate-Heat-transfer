#!/bin/bash -l

module load intel likwid

make clean

echo "WITH LIKWID"
LIKWID=on CXX=icpx make -j > /dev/null

./test


make clean

echo "NO LIKWID"
LIKWID=off CXX=icpx make -j > /dev/null

./test

make cleanall