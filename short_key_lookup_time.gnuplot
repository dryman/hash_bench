set term png size 720,640
set output 'short_key_lookup_time.png'
set grid
set title 'Hash Table Lookup Time'
set datafile separator ','
set key autotitle columnhead
set xlabel 'Number of items inserted'
plot 'short_key_lookup_time.csv' u 1:2 w lp, '' u 1:3 w lp, '' u 1:4 w lp, '' u 1:5 w lp, '' u 1:6 w lp, '' u 1:7 w lp
