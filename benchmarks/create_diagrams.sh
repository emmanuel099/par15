#!/bin/bash

mkdir -p .gptmp

for benchmark in *benchmark.*.csv; do
    rm -Rf ".gptmp/*"
    awk '/rows/{filename=".gptmp/tmp"NR".csv"}; {print >filename}' "${benchmark}"
    for b in .gptmp/tmp*.csv; do
        echo "${b}"
        gnuplot -e "infile='${b}'; outname='${benchmark%.*}'" "bench.gnuplot"
    done
done

rm -Rf ".gptmp"

exit 0