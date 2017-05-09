#!/bin/bash

#echo "Processone"

# Script for processing results
T=$1

ERR=`dirname $T`/err.log
touch $ERR
#rm $ERR

OUT=`dirname $T`/out.log
touch $OUT
#rm $OUT

missing=""

#echo "input file: $T"
#echo "out: $OUT"

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
    echo "Missing $missing: $T" >> $ERR
    exit 2
fi

nsolved=$(grep "solvetime" $T | wc -l | grep -o "[0-9]")

if [ $nsolved != 4 ]
then
    echo "Solved $nsolved: $T" >> $ERR
    exit 2
fi

prop=(reg tup_mem tup_speed ct)

count=0
for time in `grep "solvetime" $T | grep -o "([0-9]*.[0-9][0-9][0-9]" | cut -d "(" -f2`; do
    echo "${prop[$count]}($time)" >> $OUT
    count=$(($count+1))
    if (( $count == 4))
    then
	exit 2
    fi
done





