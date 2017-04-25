#!/bin/sh

# Script for processing results
T=$1

ERR="err.log"
touch $ERR

OUT="out.log"
touch $OUT

missing=""

grep -q "REGULAR" $T ||
    {
        missing="$missing reg"
    }
grep -q "TUPLESET_SPEED" $T ||
    {
        missing="$missing tup_speed"
    }

grep -q "TUPLESET_MEM" $T ||
    {
        missing="$missing tup_mem"
    }

grep -q "COMPACT" $T ||
    {
        missing="$missing ct"
    }

#do
if [ "$missing" != "" ]
then
    echo "Missing $missing: $T" >> err.log
    exit 2
fi

nsolved=$(grep "solvetime" $T | wc -l | grep -o "[0-9]")

if [ $nsolved != 4 ]
then
    echo "Solved $nsolved: $T" >> err.log
    exit 2
fi

prop[0]="reg"
prop[1]="tup_mem"
prop[1]="tup_speed"
prop[2]="ct"

count=0

for time in `grep "solvetime" $T | grep -o "([0-9]*.[0-9][0-9][0-9]" | cut -d "(" -f2`; do
    echo "${prop[$count]}($time)" >> $OUT
    let count=$count+1
done





