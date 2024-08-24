#!/bin/bash -l

module load intel likwid

make clean

likwid=off CXX=icpx make -j > /dev/null

./test
