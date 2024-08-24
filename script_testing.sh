#!/bin/bash -l

module load intel likwid

likwid=off CXX=icpx make -j > /dev/null

./test
