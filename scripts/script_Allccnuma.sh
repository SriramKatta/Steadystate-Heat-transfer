#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --partition=singlenode 
#SBATCH --output=./SLURM_OUT_FILES/%j_%x.out
#SBATCH --time=00:59:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

numdoma=$(lscpu | grep "NUMA node(s)" | awk '{print $NF}')
procs=$(nproc)
cpustep=$(echo "$procs / $numdoma" | bc)
cpustart=$cpustep
numacpustart=$(echo "$cpustart + $cpustep" | bc)

module load intel 
module load likwid 

simdir="simdataAllccnuma"

[ ! -d $simdir ] && mkdir $simdir

DATE=$(date +'%d-%m-%y_%H@%M@%S')
perffile=perf_$DATE

LIKWID=off CXX=icpx make > /dev/null
make clean

cp perf $perffile

for simrange in "2000 20000" #"20000 2000" "1000 400000"
do
outplotname=$(echo $simrange | awk '{print $(NF)}')
cgfname=./$simdir/cgperf_$outplotname
pcgfname=./$simdir/pcgperf_$outplotname
echo "#numthreds perf" > $cgfname
echo "#numthreds perf" > $pcgfname

echo "---------------------------------------------------------------------------------------"
echo "simrange : $simrange"
echo "---------------------------------------------------------------------------------------"

for numcores in $(seq 1 $cpustart) $(seq $numacpustart $cpustep $procs)
do
echo -n "numcores $numcores $simrange : "
    srun --cpu-freq=2000000-2000000:performance likwid-pin -q -c E:N:$numcores ./$perffile ${simrange} > procfile
    numthreads=$(cat procfile | grep -i "threads active" | awk '{print $(NF)}')
    cgperf=$(cat procfile | grep -i "Performance CG" | awk '{print $(NF-1)}')
    pcgperf=$(cat procfile | grep -i "Performance PCG" | awk '{print $(NF-1)}')
    rm procfile
    echo "$numthreads $cgperf" >> $cgfname
    echo "$numthreads $pcgperf" >> $pcgfname
echo "$numthreads $cgperf $pcgperf"
done
done
gnuplot ./plotscript/perf_cg_Allcc.gnuplot
gnuplot ./plotscript/perf_pcg_Allcc.gnuplot

rm $perffile
make clean

