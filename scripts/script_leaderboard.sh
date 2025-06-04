#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --partition=singlenode
#SBATCH --output=./SLURM_OUT_FILES/%j_%x.out
#SBATCH --time=00:10:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

numdoma=$(lscpu | grep "NUMA node(s)" | awk '{print $NF}')
procs=$(nproc)
cpustep=$(echo "$procs / $numdoma" | bc)
cpustart=$cpustep
numacpustart=$(echo "$cpustart + $cpustep" | bc)

module load intel
module load likwid

DATE=$(date +'%d-%m-%y_%H@%M@%S')
perffile=perf_$DATE

LIKWID=off CXX=icpx make >/dev/null
make clean

cp perf $perffile

for numcores in $(seq $numacpustart $cpustep $procs); do
    ./leaderboard.sh "" "likwid-pin -q -c E:N:$procs"
done

rm $perffile
make clean
