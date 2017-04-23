#!/bin/sh

# Script for processing results
T=$1

ERR="err.log"
touch $ERR

OUT="out.log"
touch $OUT

grep -q "COMPACT" $T &&
    grep -q "REGULAR" $T &&
    grep -q "TUPLESET" $T ||
        {
            echo "$1" >> err.log
            exit 2
        }

prop[0]="reg"
prop[1]="tup"
prop[2]="ct"

count=0

echo $T >> $OUT

for time in `grep "solvetime" $T | grep -o "([0-9]*.[0-9][0-9][0-9]" | cut -d "(" -f2`; do
    echo "${prop[$count]}($time)" >> $OUT
    let count=$count+1
done
