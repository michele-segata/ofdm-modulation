#set terminal postscript enhanced eps color
set terminal pdf
#set term tikz standalone color solid size 5in,3in
#set output "samples.tex"
set output "samples.pdf"
plot "samples.dat" using 1:($1>=0 && $1 <= 161 ? $2 : 1/0) with lines title "OFDM Short Training Sequence", \
     "samples.dat" using 1:($1>=161 && $1 <= 193 ? $2 : 1/0) with lines title "Cyclic prefix", \
     "samples.dat" using 1:($1>=193 && $1 <= 320 ? $2 : 1/0)  with lines title "OFDM Long Training Sequence"
