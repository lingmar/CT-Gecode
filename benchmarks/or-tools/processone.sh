#!/bin/bash

#echo "Processone"

# Script for processing results
T=$1
dir=`dirname $T`

ERR=$dir/err.log
touch $ERR
#rm $ERR
prop=(dfa b i ct)
echo "input file: $T"

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
    echo "Missing $missing: $T" >> $ERR
    exit 2
fi

nsolved=$(grep "solvetime" $T | wc -l | grep -o "[0-9]")

if [ $nsolved != 4 ]
then
    echo "Solved $nsolved: $T" >> $ERR
    exit 2
fi

count=0
for time in `grep "solvetime" $T | grep -o "([0-9]*.[0-9][0-9][0-9]" | cut -d "(" -f2`; do
    echo "$time" >> $dir/${prop[$count]}-solve.data
    count=$(($count+1))
done

count=0
for time in `grep "runtime" $T | grep -o "([0-9]*.[0-9][0-9][0-9]" | cut -d "(" -f2`; do
    echo "$time" >> $dir/${prop[$count]}-run.data
    count=$(($count+1))
done





