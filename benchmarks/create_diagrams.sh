#!/bin/bash

for benchmark in *benchmark.*.csv; do
    mkdir -p .gptmp
    awk '/rows/{filename=".gptmp/tmp"NR".csv"}; {print >filename}' "${benchmark}"
    for b in .gptmp/tmp*.csv; do
        echo "${b}"
        gnuplot -e "infile='${b}'; outname='${benchmark%.*}'" "bench.gnuplot"
    done
    rm -Rf ".gptmp"
done

exit 0