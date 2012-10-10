set terminal pdf 
set output "ofdm.pdf"
set xrange [0:881]
set yrange [-0.3:0.3]
set xlabel "Sample index (#)"

set style line 1 lw 2 
set style line 2 lw 2
set style line 3 lw 2
set style line 4 lw 2
set style line 5 lw 2

set grid

set multiplot

set size 1,0.45
set origin 0,0.40
set key off
set ylabel "Amp (real) (#)"
plot "ofdm.dat" using 1:($1>=0 && $1 <= 161 ? $2 : 1/0) with lines title "Short Training Sequence" ls 1, \
     "ofdm.dat" using 1:($1>=161 && $1 <= 193 ? $2 : 1/0) with lines title "Cyclic prefix" ls 2, \
     "ofdm.dat" using 1:($1>=193 && $1 <= 320 ? $2 : 1/0)  with lines title "Long Training Sequence" ls 3, \
     "ofdm.dat" using 1:($1>=320 && $1 <= 336 ? $2 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=336 && $1 <= 400 ? $2 : 1/0)  with lines title "SIGNAL header" ls 4, \
     "ofdm.dat" using 1:($1>=400 && $1 <= 416 ? $2 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=416 && $1 <= 480 ? $2 : 1/0)  with lines title "Data symbol" ls 5, \
     "ofdm.dat" using 1:($1>=480 && $1 <= 496 ? $2 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=496 && $1 <= 560 ? $2 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=560 && $1 <= 576 ? $2 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=576 && $1 <= 640 ? $2 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=640 && $1 <= 656 ? $2 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=656 && $1 <= 720 ? $2 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=720 && $1 <= 736 ? $2 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=736 && $1 <= 800 ? $2 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=800 && $1 <= 816 ? $2 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=816 && $1 <= 881 ? $2 : 1/0)  with lines notitle ls 5

set key off
set yrange [-0.30:0.30]

set size 1,0.45
set origin 0,0

set ylabel "Amp (imaginary) (#)"
plot "ofdm.dat" using 1:($1>=0 && $1 <= 161 ? $3 : 1/0) with lines title "Short Training Sequence" ls 1, \
     "ofdm.dat" using 1:($1>=161 && $1 <= 193 ? $3 : 1/0) with lines title "Cyclic prefix" ls 2, \
     "ofdm.dat" using 1:($1>=193 && $1 <= 320 ? $3 : 1/0)  with lines title "Long Training Sequence" ls 3, \
     "ofdm.dat" using 1:($1>=320 && $1 <= 336 ? $3 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=336 && $1 <= 400 ? $3 : 1/0)  with lines title "SIGNAL header" ls 4, \
     "ofdm.dat" using 1:($1>=400 && $1 <= 416 ? $3 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=416 && $1 <= 480 ? $3 : 1/0)  with lines title "Data symbol" ls 5, \
     "ofdm.dat" using 1:($1>=480 && $1 <= 496 ? $3 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=496 && $1 <= 560 ? $3 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=560 && $1 <= 576 ? $3 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=576 && $1 <= 640 ? $3 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=640 && $1 <= 656 ? $3 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=656 && $1 <= 720 ? $3 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=720 && $1 <= 736 ? $3 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=736 && $1 <= 800 ? $3 : 1/0)  with lines notitle ls 5, \
     "ofdm.dat" using 1:($1>=800 && $1 <= 816 ? $3 : 1/0)  with lines notitle ls 2, \
     "ofdm.dat" using 1:($1>=816 && $1 <= 881 ? $3 : 1/0)  with lines notitle ls 5

#key
unset tics
unset border
set yrange [0:1]
set key tmargin
set size 1,0.3
set origin 0,0.7
unset xlabel
unset ylabel
set key maxrows 3
set key font "Helvetica,5"
set key tmargin 
plot -2 with lines ls 1 title "Short Tranining Sequence", \
		 -2 with lines ls 2 title "Cyclic Prefix", \
		 -2 with lines ls 3 title "Long Training Sequence", \
		 -2 with lines ls 4 title "SIGNAL Header", \
		 -2 with lines ls 5 title "Data Symbol"

unset multiplot
