#!/bin/bash

dir=$1

for m in "NOTHING" "DELTA" "LONG_FILTER" "FIX"; do
    echo $m
    ./fzn-define-macro.sh $m
    find instances/$dir -name "*.fzn" | parallel --gnu -P 4 --progress nice ./comparectone.sh {} $m
    echo "*******************************************"
done

