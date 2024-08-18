#!/bin/bash -l

module load intel likwid

make clean

likwid=on CXX=icpx make -j > /dev/null

./test

make clean