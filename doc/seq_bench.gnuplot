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

set terminal pdf color enhanced font "Roboto,12"
set output sprintf("%s_%s.pdf", outname, getValue("its", 2))

set grid
set xlabel 'Matrix Size (height and width)'
set xtics out
set ylabel 'Time in milliseconds'
set ytics out
set key left top
set logscale y

load "dark2.pal"

algorithms=system("awk -F';' 'NR>4 {a[$1];}END{for (i in a) print i;}' ".infile)
i = 0
plot for [algorithm in algorithms]\
    getDataOfCategory(algorithm)\
    using 1:3\
    with linespoints\
    title getTitle(algorithm)\
    ls i = i + 1\
    lw 2 