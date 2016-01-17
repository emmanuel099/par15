#!/bin/bash

rows=$1
cols=$2
its=$3
threads=$4

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
exit 0