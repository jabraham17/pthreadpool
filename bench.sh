#!/bin/bash
for N in 1 2 4 8 16
do
    make -B THREADS=$N --quiet
    echo "$N Threads"
    ./test.out
    echo ""
    make -B VECTORIZE=1 THREADS=$N --quiet
    echo "$N Threads"
    ./test.out
    echo ""
done    
make clean --quiet
