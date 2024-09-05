#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --partition=singlenode
#SBATCH --cpus-per-task=72
#SBATCH --time=00:30:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

module load intel likwid


LIKWID=off CXX=icpx make -j > /dev/null

#DATE=$(date +'%d-%m-%y_%H@%M@%S')
#fname=datafile_${DATE}
#touch $fname

for simrange in "2000 20000" "20000 2000" #"1000 400000"
do
    srun --cpu-freq=2000000-2000000:performance \
            likwid-pin -q -c N:0-17 \
            ./perf ${simrange}
done


make clean