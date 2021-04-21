#!/bin/bash
for N in 1 2 4 8 16
do
    make -B THREADS=$N VECTORIZE=1 ACCURACY=0 --quiet -j2
    echo "$N Threads"
    ./bench.out
    echo ""
done    
make clean --quiet
