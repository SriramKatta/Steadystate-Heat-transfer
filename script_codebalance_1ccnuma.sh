#!/bin/bash -l
#
#SBATCH --nodes=1
#SBATCH --partition=singlenode
#SBATCH --constraint=hwperf
#SBATCH --cpus-per-task=72
#SBATCH --time=00:30:00
#SBATCH --export=NONE

unset SLURM_EXPORT_ENV

module load intel 
module load likwid 


[ ! -d codebalance_data ] && mkdir codebalance_data

make clean

#LIKWID=on CXX=icpx make -j > /dev/null

region=""
intensity=""

for simrange in "2000 20000" "20000 2000" "1000 400000"
do
    outfilename=./codebalance_data/file$(echo $simrange | awk '{print $(NF)}')
    srun --cpu-freq=2000000-2000000:performance likwid-perfctr -m -C M0:0-17 -g MEM_DP ./perf $simrange > ./ codebalancedata/$outfilename
echo "---------------------------------------------------------------------------------------"
echo "simrange : $simrange"
echo "---------------------------------------------------------------------------------------"

    while IFS= read -r line; do
    # Detect and extract region names
    if [[ $line == Region* ]]; then
        region=$(echo $line | awk '{print $2}')
    fi

    # Detect and extract operational intensity values
    if [[ $line == *"Operational intensity [FLOP/Byte] STAT"* ]]; then
        intensity=$(echo $line | awk -F'|' '{print $3}')
        
        # Output region and intensity
        echo "Region: $region, Operational Intensity: $intensity [FLOP/Byte], Code balance: $(echo "1/$intensity" | bc -l) [Byte/FLOP]"
    fi
done < "$outfilename"

done


make clean