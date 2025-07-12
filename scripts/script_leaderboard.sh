#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --partition=singlenode
#SBATCH --output=./SLURM_OUT_FILES/%j_%x.out
#SBATCH --time=00:10:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

module load intel
module load likwid

DATE=$(date +'%d-%m-%y_%H@%M@%S')
perffile=perf_$DATE

make clean
LIKWID=off CXX=icpx make -j >/dev/null

cp perf $perffile

./leaderboard.sh "" "likwid-pin -q -c E:N:$(nproc)"

rm $perffile
make clean
