#!/bin/bash

# Input file path
outfilename="file20000"

# Initialize variables


# Read the file line by line
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

