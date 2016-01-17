#!/bin/bash
# usage: ./benchmark_jupiter.sh "1000 2000;50 50" 10 "1;2;4;8"

test_sizes=$1
its=$2
threads=$3

hostfile="hosts"
ts=$(date +%s)
out="benchmark.${ts}.csv"

cmds_sequential=(
"build/stencil_sequential/sequential_benchmark_tmp_matrix"
"build/stencil_sequential/sequential_benchmark_one_vector"
)

cmds=(
"build/stencil_mpi/mpi_benchmark_sendrecv"
"build/stencil_mpi/mpi_benchmark_nonblocking"
"build/stencil_mpi/mpi_benchmark_onesided_fence"
"build/stencil_mpi/mpi_benchmark_onesided_pscw"
)

for test_size in ${test_sizes[@]}; do
    read rows cols <<< "$test_size"

    echo "rows;${rows}" > ${out}
    echo "cols;${cols}" >> ${out}
    echo "its;${its}" >> ${out}
    echo "P;${threads}" >> ${out}

    # sequential
    for cmd in ${cmds_sequential[@]}; do
        output=$(${sequential} ${rows} ${cols} ${its})
        echo "${cmd};1;${output}" >> ${out}
    done

    # mpi
    for cmd in ${cmds[@]}; do
        for par in $(echo $threads | tr ";" "\n"); do
            output=$(/opt/openmpi/bin/mpirun -np ${par} --hostfile ${hostfile} ${cmd} ${rows} ${cols} ${its})
            echo "${cmd};${par};${output}" >> ${out}
        done
    done
done

exit 0