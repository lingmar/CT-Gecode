# Script for running fzn-gecode
T=$1
unlzma $T

FZN=${T%.*}

OUT=$(echo $FZN | rev | cut -f 2- -d '.' | rev).res

for prop in {"gecode-regular","gecode-tupleset","compact-table"}; do
    export TABLE_PROPAGATOR=$prop
    ${GECODE_PATH}/bin/fzn-gecode -s $FZN >> $OUT
done

# Compress fzn file 
lzma $FZN 
