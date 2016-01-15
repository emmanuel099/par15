#!/bin/bash

rows=$1
cols=$2
its=$3
threads=$4

hostfile="hosts"
out="benchmark.csv"
sequential="build/stencil_sequential/sequential_benchmark"

cmds=(
"build/stencil_mpi/mpi_benchmark_sendrecv"
"build/stencil_mpi/mpi_benchmark_nonblocking"
"build/stencil_mpi/mpi_benchmark_onesided"
)

echo "P;${threads}" > ${out}

# sequential
time=$(${sequential} ${rows} ${cols} ${its})
echo "sequential;${time}" >> ${out}

# mpi
for cmd in ${cmds[@]}; do
    output="${cmd};";
    for par in $(echo $threads | tr ";" "\n"); do
        time=$(/opt/openmpi/bin/mpirun -np ${par} --hostfile ${hostfile} ${cmd} ${rows} ${cols} ${its})
        output="${output}${time};"  
    done
    echo ${output} >> ${out}
done
exit 0