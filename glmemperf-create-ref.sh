#!/bin/sh
# Script for generating GLMemPerf reference frames

TESTS=`glmemperf -l|tail -n +4|grep -v ^Warning:`

for TEST in $TESTS; do
    echo "Generating reference frame $TEST.raw"
    glmemperf -t 4 -i $TEST &
    sleep 2
    cp /dev/fb0 $TEST.raw
    wait
done
