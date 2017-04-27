#!/bin/sh

rm `find . -name "*.log"`
rm `find . -name "*.data"`

find . -name "*.res" | parallel -P 16 --progress nice ./processone.sh {}

REG="reg.log"
TUP_SPEED="tup_speed.log"
TUP_MEM="tup_mem.log"
CT="ct.log"

RAW="out.log"
OUT=$(echo $f | cut -d '.' -f1).data

for dir in `ls instances`; do
    touch $dir/$REG
    touch $dir/$TUP_MEM
    touch $dir/$TUP_SPEED
    touch $dir/$CT

    rm $dir/$REG
    rm $dir/$TUP_MEM
    rm $dir/$TUP_SPEED
    rm $dir/$CT

    cat out.log | grep "reg" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $REG
    cat out.log | grep "tup_mem" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $TUP_MEM
    cat out.log | grep "tup_speed" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $TUP_SPEED
    cat out.log | grep "ct" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $CT

    for f in $dir/$REG $dir/$TUP_MEM $dir/$TUP_SPEED $dir/$CT; do
        count=1
        for line in `cat $f`; do
            echo "$line $count" >> "$dir/$OUT"
	    count=$(($count+1))
        done
    done

    ./generate-graph.sh $dir
done



