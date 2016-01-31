#!/bin/bash

for benchmark in *.csv; do
    mkdir -p .gptmp
    awk '/rows/{filename=".gptmp/tmp"NR".csv"}; {print >filename}' "${benchmark}"
    for b in .gptmp/tmp*.csv; do
        if echo "${benchmark}" | grep -q "seq"; then
            gnuplot -e "infile='${b}'; outname='${benchmark%.*}'" "seq_bench.gnuplot"
        elif echo "${benchmark}" | grep -q "scale"; then
            gnuplot -e "infile='${b}'; outname='${benchmark%.*}'" "scale.gnuplot"
        else
            gnuplot -e "infile='${b}'; outname='${benchmark%.*}'" "bench.gnuplot"
        fi
    done
    rm -Rf ".gptmp"
done

exit 0