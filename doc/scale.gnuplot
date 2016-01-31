getDataOfCategory(cat)=(\
    sprintf("< awk -F';' '$1==\"%s\" { print $2,$3 }' %s", cat, infile)\
)

set terminal pdf color enhanced font "Roboto,12"
set output sprintf("%s.pdf", outname)

set grid
set xlabel 'Threads/Nodes'
set xtics 4 out
set ylabel 'Scalability'
set ytics out
set key below

load "dark2.pal"

# horizontal line at scalability=1
set arrow from graph 0,first 1 to graph 1,first 1 nohead dashtype 4 front

algorithms=system("awk -F';' 'NR>4 {a[$1];}END{for (i in a) print i;}' ".infile)
i = 0
plot for [algorithm in algorithms]\
    getDataOfCategory(algorithm)\
    using 1:($2/$1)\
    with linespoints\
    title algorithm\
    ls i = i + 1\
    lw 2