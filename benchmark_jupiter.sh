#!/bin/bash

rows=$1
cols=$2
its=$3
threads=$4

hostfile="hosts"
ts=$(date +%s)
out="benchmark.${ts}.csv"
sequential="build/stencil_sequential/sequential_benchmark"

cmds=(
"build/stencil_mpi/mpi_benchmark_sendrecv"
"build/stencil_mpi/mpi_benchmark_nonblocking"
"build/stencil_mpi/mpi_benchmark_onesided"
)

echo "rows;${rows}" > ${out}
echo "cols;${cols}" >> ${out}
echo "its;${its}" >> ${out}
echo "P;${threads}" >> ${out}

# sequential
time=$(${sequential} ${rows} ${cols} ${its})
echo "sequential;${time}" >> ${out}

# mpi
for cmd in ${cmds[@]}; do
    for par in $(echo $threads | tr ";" "\n"); do
        output=$(/opt/openmpi/bin/mpirun -np ${par} --hostfile ${hostfile} ${cmd} ${rows} ${cols} ${its})
        echo "${cmd};${par};${output};" >> ${out}
    done
done
exit 0