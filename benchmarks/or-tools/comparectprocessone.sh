#!/bin/bash

#echo "Processone"

# Script for processing results
T=$1
N=$(echo $f | cut -d '.' -f1)-NOTHING.res
D=$(echo $f | cut -d '.' -f1)-DELTA.res
L=$(echo $f | cut -d '.' -f1)-LONG_FILTER.res
F=$(echo $f | cut -d '.' -f1)-FIX.res

ERR=`dirname $T`/err.log
touch $ERR
#rm $ERR

OUT=`dirname $T`/out.log
touch $OUT
#rm $OUT

missing=""

#echo "input file: $T"
#echo "out: $OUT"

[ -f $N ] && [ -f $D ] && [ -f $L ] && [ -f $F ] || echo "Some variant is missing for $T!" && exit 2   

prop=(n d l f)

count=0
for f in $N $D $L $F; do
    time=$(grep "solvetime" $f | grep -o "([0-9]*.[0-9][0-9][0-9]" | cut -d "(" -f2)
    echo "${prop[$count]}($time)" >> $OUT
    count=$(($count+1))
done





