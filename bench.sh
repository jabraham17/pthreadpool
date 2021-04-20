#!/bin/bash
for N in 1 2 4 8 16
do
    make -B CFLAGS="-DNUM_THREADS=$N" --quiet
    echo "$N Threads"
    ./test.out
    echo ""
done    
make clean --quiet
