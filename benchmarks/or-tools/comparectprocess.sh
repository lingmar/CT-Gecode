#!/bin/bash

DIR=$1

find instances/$DIR -name "*.fzn" | parallel --gnu -P 4 --progress nice ./comparectprocessone.sh {}

echo "Done processone"

NOTHING="nothing.log"
DELTA="delta.log"
LONG_FILTER="long_filter.log"
FIX="fix.log"

dir=instances/$
echo $dir

cat $dir/out.log | grep "n" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$NOTHING
cat $dir/out.log | grep "d" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$DELTA
cat $dir/out.log | grep "l" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$LONG_FILTER
cat $dir/out.log | grep "f" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$FIX

prop=(n d l f)
index=0

for f in $dir/$NOTHIN $dir/$DELTA $dir/$LONG_FILTER $dir/$FIX; do
    count=1
    OUT="${prop[index]}.data"
    rm $dir/$OUT
    for line in `cat $f`; do
        echo "($line, $count)" >> "$dir/$OUT"
	count=$(($count+1))
    done
    index=$(($index+1))
done

#dirname `find . -name "ct.data"` | parallel --gnu -P 8 --progress nice ./generate-graph.sh {}

