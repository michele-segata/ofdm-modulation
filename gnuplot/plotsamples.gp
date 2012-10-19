set terminal pdf
set output "samples.pdf"

set yrange [-0.3:0.3]
set style line 1 lw 2 lc rgb "#1978AE"
set style line 1 lw 2 lc rgb "#BFEF7F"

set style line 50 lc rgb "#FFFFFF" lw 2

set border ls 50

set multiplot

set origin 0,0.5
set size 1,0.5

set object 1 rectangle from screen -1,0.5 to screen 2,2 fillcolor rgb "black" behind

plot "samples.dat" using 1:2 with lines notitle ls 1

set origin 0,0
set size 1,0.5

set object 2 rectangle from screen -1,0 to screen 2,0.5 fillcolor rgb "black" behind
unset object 1
plot "samples.dat" using 1:3 with lines notitle ls 1

unset multiplot
