getTitle(algorithm)=(\
    (algorithm eq "build/stencil_sequential/sequential_benchmark_tmp_matrix") ? "Sequential (Matrix)" :\
    (algorithm eq "build/stencil_sequential/sequential_benchmark_one_vector") ? "Sequential (Vector)" :\
    algorithm\
)

getValue(name, col)=(\
    system(sprintf("awk -F';' '$1==\"%s\" { print $%i; exit }' %s", name, col, infile))\
)

getDataOfCategory(cat)=(\
    sprintf("< awk -F';' '$1==\"%s\" { print $2,$3,$4,$5 }' %s", cat, infile)\
)

outfile=sprintf("%s_%sx%s.pdf", outname, getValue("rows", 2), getValue("cols", 2))
set terminal postscript color
set output '| ps2pdf - '.outfile

set grid
set xlabel 'Iterations'
set xtics 4 out
set ylabel 'Time in milliseconds'
set ytics out
set key left top

algorithms=system("awk -F';' 'NR>4 {a[$1];}END{for (i in a) print i;}' ".infile)
plot for [algorithm in algorithms]\
    getDataOfCategory(algorithm)\
    using 1:2\
    with linespoints\
    title getTitle(algorithm)