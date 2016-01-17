#!/bin/bash

rows=$1
cols=$2
its=$3
threads=$4

ts=$(date +%s)
out="benchmark.${ts}.csv"
sequential="build/stencil_sequential/sequential_benchmark"

cmds=(
"build/stencil_cilk/cilk_benchmark"
"build/stencil_openmp/openmp_benchmark"
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
time=$(${sequential} ${rows} ${cols} ${its})
echo "sequential;${time}" >> ${out}

# cilk / openmp
for cmd in ${cmds[@]}; do
    for par in $(echo $threads | tr ";" "\n"); do
        output=$(${cmd} ${rows} ${cols} ${its} ${par})
        echo "${cmd};${par};${output}" >> ${out}
    done
done
exit 0