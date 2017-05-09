#!/bin/bash

#rm `find . -name "*.log"`
#rm `find . -name "*.data"`

#find . -name "*.res" | tail -n +$1 | head -n +$2 | parallel -P 16 --progress nice ./processone.sh {}

echo "Done processone"

REG="reg.log"
TUP_SPEED="tup_speed.log"
TUP_MEM="tup_mem.log"
CT="ct.log"

RAW="out.log"
OUT=$(echo $f | cut -d '.' -f1).data

#for dir in "./instances/randsJC2500"; do
for dir in `ls instances`; do
    dir=instances/$dir
    echo $dir
    #touch $dir/$REG
    #touch $dir/$TUP_MEM
    #touch $dir/$TUP_SPEED
    #touch $dir/$CT

    #rm $dir/$REG
    #rm $dir/$TUP_MEM
    #rm $dir/$TUP_SPEED
    #rm $dir/$CT

    #cat $dir/out.log | grep "reg" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n >$dir/$REG
    #cat $dir/out.log | grep "tup_mem" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$TUP_MEM
    #cat $dir/out.log | grep "tup_speed" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$TUP_SPEED
    #cat $dir/out.log | grep "ct" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$CT

    prop=(reg tup_mem tup_speed ct)
    index=0

    for f in $dir/$REG $dir/$TUP_MEM $dir/$TUP_SPEED $dir/$CT; do
        count=1
	OUT="${prop[index]}.data"
	rm $dir/$OUT
        for line in `cat $f`; do
            echo "($line, $count)" >> "$dir/$OUT"
 #           echo "$line $count -> $dir/$OUT"
	    count=$(($count+1))
        done
	index=$(($index+1))
    done

done

dirname `find . -name "ct.data"` | parallel --gnu -P 8 --progress nice ./generate-graph.sh {}

