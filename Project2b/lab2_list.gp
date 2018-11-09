#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_1.png ... throughput vs. number of threads for mutex and spin-lock synchronized list operations.
#   lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
#   lab2b_3.png ... successful iterations vs. threads for each synchronization method.
#   lab2b_4.png ... throughput vs. number of threads for mutex synchronized partitioned lists.
#   lab2b_5.png ... throughput vs. number of threads for spin-lock-synchronized partitioned lists.
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

set title "List-1: Scalability of Synchronization Mechanisms"
set xlabel "Number of Threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Throughput (ops/s)"
set logscale y
set output 'lab2b_1.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/($7)) title 'List w/mutex' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-s,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/($7)) title 'List w/spin-lock' with linespoints lc rgb 'blue'

set title "List-2: Mutex-Wait-Time and Operation Time"
set xlabel "Number of Threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Time (ns)"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):($8) title 'Wait-for-lock time' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):($7) title 'Average time per operation' with linespoints lc rgb 'blue'

set title "List-3: Successful runs with and without Synchronization"
set xlabel "Number of Threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Number of successful iterations"
set logscale y
set output 'lab2b_3.png'
set key left top
plot \
    "< grep -e 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'red' title 'unprotected', \
    "< grep -e 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'blue' title 'Mutex enabled', \
    "< grep -e 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'green' title 'Spinlock enabled', \


set title "List-4: Aggregated Throughput with Mutex"
set xlabel "Number of Threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Aggregated Throughput (Total ops/s)"
set logscale y
set output 'lab2b_4.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'red' title 'Lists = 1', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'blue' title 'Lists = 4', \
     "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'yellow' title 'Lists = 8', \
     "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'green' title 'Lists = 16'

set title "List-5: Aggregated Throughput with Spinlock"
set xlabel "Number of Threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Aggregated Throughput (Total ops/s)"
set logscale y
set output 'lab2b_5.png'
set key left top
plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'red' title 'Lists = 1', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'blue' title 'Lists = 4', \
     "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'yellow' title 'Lists = 8', \
     "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) with linespoints lc rgb 'green' title 'Lists = 16'