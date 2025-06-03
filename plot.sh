#!/bin/bash

# Check if a filename argument is provided - do for all if no file provided
if [ $# -eq 0 ]; then
    # Check if results dir exist
    if [ ! -d "results" ]; then
        echo -e "Create results first by running ./test"
        exit 1
    fi
    for file in results/*.dat; do
    	./plot.sh "$file"
    done
    exit 0
fi

if [[ $1 == "--help" || $1 == "-h" ]]; then
    echo -e 'Use this script to convert the GNU-plot .dat files into .png. Either call without arguments to convert all such files, or use ./plot.sh <filename> to only convert one file.\n\n To view the png files, either use VSCode or transfer the files to your home machine via scp(1).'
    exit 0
fi

# Get the filename from the first argument
data_file="$1"

# Check if the file exists
if [ ! -f "$data_file" ]; then
    echo "Error: File '$data_file' not found."
    exit 1
fi

# Run Gnuplot with the provided data file
gnuplot <<EOF
set terminal pngcairo size 800,600 enhanced font "Arial,12"
set output "${data_file%.*}.png"
set view map
set size ratio .9
set object 1 rect from graph 0, graph 0 to graph 1, graph 1 back
set object 1 rect fc rgb "black" fillstyle solid 1.0
unset key
splot "$data_file" using 1:2:3 with points pointtype 5 pointsize 1 palette linewidth 30
EOF

