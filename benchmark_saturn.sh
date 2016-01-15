#!/bin/bash

rows=$1
cols=$2
its=$3
threads=$4

out="benchmark.csv"
sequential="build/stencil_sequential/sequential_benchmark"

cmds=(
"build/stencil_cilk/cilk_benchmark"
"build/stencil_openmp/openmp_benchmark"
)

echo "P;${threads}" > ${out}

# sequential
time=$(${sequential} ${rows} ${cols} ${its})
echo "sequential;${time}" >> ${out}

# cilk / openmp
for cmd in ${cmds[@]}; do
    output="${cmd};";
    for par in $(echo $threads | tr ";" "\n"); do
        time=$(${cmd} ${rows} ${cols} ${its} ${par})
        output="${output}${time};"  
    done
    echo ${output} >> ${out}
done
exit 0