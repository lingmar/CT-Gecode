#!/bin/sh

# Script for processing results
T=$1

echo $1

correct=0
processed=0

if [[!(grep -q "COMPACT" "$T") || !(grep -q "REGULAR" "$T")]]; then
    echo "Woo"
else
    echo "ooW"
fi

for time in grep "solvetime"; do
    echo $time
done

            
# unlzma $T

# FZN=${T%.*}

# OUT=$(echo $FZN | rev | cut -f 2- -d '.' | rev).res
# touch $OUT

# for prop in {"gecode-regular","gecode-tupleset","compact-table"}; do
#     export TABLE_PROPAGATOR=$prop
#     ${GECODE_PATH}/bin/fzn-gecode -s $FZN >> $OUT
# done

# # Compress fzn file 
# lzma $FZN 
