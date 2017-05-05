#!/bin/bash

T=$1
DIR=instances/$T

find $DIR -name "*.fzn.lzma" | parallel --gnu -P 4 --progress nice ./comparectprocessone.sh {}

echo "Done processone"

NOTHING="nothing.log"
DELTA="delta.log"
LONG_FILTER="long_filter.log"
FIX="fix.log"

cat $DIR/out.log | grep "n" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $DIR/$NOTHING
cat $DIR/out.log | grep "d" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $DIR/$DELTA
cat $DIR/out.log | grep "l" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $DIR/$LONG_FILTER
cat $DIR/out.log | grep "f" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $DIR/$FIX

prop=(n d l f)
index=0

for f in $DIR/$NOTHING $DIR/$DELTA $DIR/$LONG_FILTER $DIR/$FIX; do
    count=1
    OUT="${prop[index]}.data"
    touch $DIR/$OUT
    rm $DIR/$OUT
    for line in `cat $f`; do
        echo "($line, $count)" >> "$DIR/$OUT"
	count=$(($count+1))
    done
    index=$(($index+1))
done

#DIRname `find . -name "ct.data"` | parallel --gnu -P 8 --progress nice ./generate-graph.sh {}

