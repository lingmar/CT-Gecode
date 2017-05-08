#!/bin/bash
# Script for running fzn-gecode

echo "hej"
FZN=$1
#unlzma $T

#FZN=${T%.*}

OUT=$(echo $FZN | rev | cut -f 2- -d '.' | rev).res
touch $OUT
rm $OUT
touch $OUT

for prop in "gecode-regular" "gecode-tupleset-mem" "gecode-tupleset-speed" "compact-table"; do
    export TABLE_PROPAGATOR=$prop
    ${GECODE_PATH}/bin/fzn-gecode -time 1000000 -s $FZN >> $OUT
done

# Compress fzn file 
#lzma $FZN
