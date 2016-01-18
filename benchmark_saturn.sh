#!/bin/bash
# usage: ./benchmark_saturn.sh "1000,2000;50,50" 10 "1;2;4;8"

test_sizes=$1
its=$2
threads=$3

ts=$(date +%s)
out="benchmark.${ts}.csv"

cmds_sequential=(
"build/stencil_sequential/sequential_benchmark_tmp_matrix"
"build/stencil_sequential/sequential_benchmark_one_vector"
)

cmds=(
"build/stencil_cilk/cilk_benchmark"
"build/stencil_openmp/openmp_benchmark_tmp_matrix"
"build/stencil_openmp/openmp_benchmark_one_vector"
"build/stencil_mpi/mpi_benchmark_sendrecv"
"build/stencil_mpi/mpi_benchmark_nonblocking"
"build/stencil_mpi/mpi_benchmark_onesided_fence"
"build/stencil_mpi/mpi_benchmark_onesided_pscw"
)

for test_size in $(echo $test_sizes | tr ";" "\n"); do
    IFS=",";
    tmp=($test_size);
    rows=${tmp[0]}
    cols=${tmp[1]}
    IFS=$'\n'
    
    echo "rows;${rows}" >> ${out}
    echo "cols;${cols}" >> ${out}
    echo "its;${its}" >> ${out}
    echo "algorithm;P;min;avg;max" >> ${out}

    # sequential
    for cmd in ${cmds_sequential[@]}; do
        output=$(${cmd} ${rows} ${cols} ${its})
        echo "${cmd};1;${output}" >> ${out}
        echo "${cmd};1;${output}"
    done

    # cilk / openmp / mpi
    for cmd in ${cmds[@]}; do
        for par in $(echo $threads | tr ";" "\n"); do
            output=$(${cmd} ${rows} ${cols} ${its} ${par})
            echo "${cmd};${par};${output}" >> ${out}
            echo "${cmd};${par};${output}"
        done
    done

    echo "" >> ${out}
done

exit 0