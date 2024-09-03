#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --partition=singlenode
#SBATCH --constraint=hwperf
#SBATCH --cpus-per-task=72
#SBATCH --time=01:30:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

module load intel 
module load likwid 


[ ! -d simdata ] && mkdir simdata

make clean

LIKWID=on CXX=icpx make -j > /dev/null

for simrange in "2000 20000" "20000 2000" "1000 400000"
do
    outplotname=$(echo $simrange | awk '{print $(NF)}')
    srun --cpu-freq=2000000-2000000:performance likwid-perfctr -m -C M0:0-18 -g MEM_DP ./perf $simrange
done


make clean