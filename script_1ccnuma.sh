#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --partition=singlenode
#SBATCH --cpus-per-task=72
#SBATCH --time=01:30:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

module load intel 
module load likwid 


[ ! -d simdata ] && mkdir simdata

make clean

LIKWID=off CXX=icpx make -j > /dev/null

for simrange in "2000 20000" "20000 2000" "1000 400000"
do
outplotname=$(echo $simrange | awk '{print $(NF)}')
cgfname=./simdata/cgperf_$outplotname
pcgfname=./simdata/pcgperf_$outplotname
echo "#numthreds perf" > $cgfname
echo "#numthreds perf" > $pcgfname

for numcores in {1..17}
do 
echo -n "start $numcores $simrange : "
    srun --cpu-freq=2000000-2000000:performance likwid-pin -q -c M0:0-$numcores ./perf ${simrange} > procfile
    numthreads=$(cat procfile | grep -i "threads active" | awk '{print $(NF)}')
    cgperf=$(cat procfile | grep -i "Performance CG" | awk '{print $(NF-1)}')
    pcgperf=$(cat procfile | grep -i "Performance PCG" | awk '{print $(NF-1)}')
    rm procfile
    echo "$numthreads $cgperf" >> $cgfname
    echo "$numthreads $pcgperf" >> $pcgfname
echo "$numthreads $cgperf $pcgperf"
done
done
gnuplot perf_cg.gnuplot
gnuplot perf_pcg.gnuplot

mv simdata simdata1ccnuma


rm procfile
make clean