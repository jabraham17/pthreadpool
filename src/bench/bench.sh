#!/bin/bash

SCRIPTPATH=$(dirname $0)

make -C $SCRIPTPATH/../../pthreadpool --quiet -j2

for N in 1 2 4 8 16
do
    make -C $SCRIPTPATH -B THREADS=$N VECTORIZE=1 ACCURACY=0 --quiet -j2
    echo "$N Threads"
    $SCRIPTPATH/bench.out
    echo ""
done    
make -C $SCRIPTPATH clean --quiet
