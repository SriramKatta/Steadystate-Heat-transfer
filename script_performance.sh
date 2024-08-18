#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --partition=singlenode
#SBATCH --cpus-per-task=72
#SBATCH --time=00:30:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

str=${1:-""}

echo "$str"

module load intel likwid

make clean

likwid=off CXX=icpx make -j > /dev/null


DATE=$(date +'%d-%m-%y_%H@%M@%S')
fname=datafile_${DATE}
touch $fname


for simrange in "2000 20000" "20000 2000" "1000 400000"
do

    srun --cpu-freq=2000000-2000000:performance \
            likwid-pin -q -c M0:0-17 \
            ./perf ${simrange} \
            | tee -a $fname \
            | grep -i "$str"

done

make clean
