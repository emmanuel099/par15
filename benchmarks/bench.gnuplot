getTitle(algorithm)=(\
    (algorithm eq "build/stencil_sequential/sequential_benchmark_tmp_matrix") ? "Sequential (Matrix)" :\
    (algorithm eq "build/stencil_sequential/sequential_benchmark_one_vector") ? "Sequential (Vector)" :\
    (algorithm eq "build/stencil_mpi/mpi_benchmark_sendrecv") ? "MPI (Sendrecv)" :\
    (algorithm eq "build/stencil_mpi/mpi_benchmark_nonblocking") ? "MPI (Nonblocking)" :\
    (algorithm eq "build/stencil_mpi/mpi_benchmark_onesided_fence") ? "MPI (Onesided-Fence)" :\
    (algorithm eq "build/stencil_mpi/mpi_benchmark_onesided_pscw") ? "MPI (Onesided-PSCW)" :\
    (algorithm eq "build/stencil_cilk/cilk_benchmark") ? "cilk" :\
    (algorithm eq "build/stencil_openmp/openmp_benchmark_tmp_matrix") ? "OpenMP (tmp matrix)" :\
    (algorithm eq "build/stencil_openmp/openmp_benchmark_one_vector") ? "OpenMP (row-wise)" :\
    (algorithm eq "build/stencil_openmp/openmp_benchmark_one_vector_tld") ? "OpenMP (row-wise, tld)" :\
    (algorithm eq "build/stencil_openmp/openmp_benchmark_one_vector_colwise") ? "OpenMP (col-wise)" :\
    (algorithm eq "build/stencil_openmp/openmp_benchmark_one_vector_colwise_tld") ? "OpenMP (col-wise, tld)" :\
    algorithm\
)

getValue(name, col)=(\
    system(sprintf("awk -F';' '$1==\"%s\" { print $%i; exit }' %s", name, col, infile))\
)

getDataOfCategory(cat)=(\
    sprintf("< awk -F';' '$1==\"%s\" { print $2,$3,$4,$5 }' %s", cat, infile)\
)

outfile=sprintf("%s_%sx%s_%s.pdf", outname, getValue("rows", 2), getValue("cols", 2), getValue("its", 2))
set terminal postscript color
set output '| ps2pdf - '.outfile

set grid
set title sprintf("Benchmark %sx%s Matrix with %s Iterations", getValue("rows", 2), getValue("cols", 2), getValue("its", 2))
set xlabel 'Threads/Nodes'
set ylabel 'Speedup'
set key left top

seqtime=getValue("build/stencil_sequential/sequential_benchmark_one_vector", 3)

algorithms=system("awk -F';' 'NR>4 {a[$1];}END{for (i in a) if (i!=\"build/stencil_sequential/sequential_benchmark_tmp_matrix\" && i!=\"build/stencil_sequential/sequential_benchmark_one_vector\") print i;}' ".infile)
plot for [algorithm in algorithms]\
    getDataOfCategory(algorithm)\
    using 1:(seqtime/$2)\
    with linespoints\
    title getTitle(algorithm)