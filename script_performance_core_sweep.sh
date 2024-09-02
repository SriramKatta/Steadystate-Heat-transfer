#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --partition=singlenode
#SBATCH --cpus-per-task=72
#SBATCH --time=01:30:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

module load intel likwid

[ ! -d simdata ] && mkdir simdata

make clean

likwid=off CXX=icpx make -j > /dev/null

for simrange in "2000 20000" "20000 2000" "1000 400000"
do
outplotname=$(echo $simrange | awk '{print $(NF)}')
cgfname=./simdata/cgperf_$outplotname
pcgfname=./simdata/pcgperf_$outplotname
echo "#numthreds perf" > $cgfname
echo "#numthreds perf" > $pcgfname

for numcores in {17..72..6}
do 
echo "start $numcores"
    srun --cpu-freq=2000000-2000000:performance \
            likwid-pin -q -c N:0-$numcores \
            ./perf ${simrange} > procfile
    numthreads=$(cat procfile | grep -i "threads active" | awk '{print $(NF)}')
    cgperf=$(cat procfile | grep -i "Performance CG" | awk '{print $(NF-1)}')
    pcgperf=$(cat procfile | grep -i "Performance PCG" | awk '{print $(NF-1)}')
    echo "$numthreads $cgperf" >> $cgfname
    echo "$numthreads $pcgperf" >> $pcgfname

done
rm procfile
done
gnuplot perf_cg.gnuplot
gnuplot perf_pcg.gnuplot


rm procfile
make clean