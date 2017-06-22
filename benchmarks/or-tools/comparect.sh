#!/bin/bash

FZN=$1
OUT=$(echo $FZN | rev | cut -f 2- -d '.' | rev).compare.res

result=""
for m in "NOTHING" "DELTA" "LONG_FILTER" "FIX"; do
    echo $m
    ./fzn-define-macro.sh $m
    # Warm up first
    export TABLE_PROPAGATOR=foo
    ${GECODE_PATH}/bin/fzn-gecode -time 300000 -s $FZN > /dev/null
    # Real run
    export TABLE_PROPAGATOR=compact-table
    t=$(${GECODE_PATH}/bin/fzn-gecode -time 1000000 -s $FZN | grep runtime | awk '{print $4}'| cut -d "(" -f2)
    result="$result$m($t);"    
    echo "*******************************************"
done

echo $result | sed 's/;$//' >> $OUT
