#!/bin/bash

dir=./instances/$1
echo $dir

rm `find . -name "*.log"`
rm `find . -name "*.data"`

#find . -name "*.res" | tail -n +$1 | head -n +$2 | parallel -P 16 --progress nice ./processone.sh {}
find $dir -name "*.res" parallel -P 4 --progress nice ./processone.sh {}

echo "Done processone"

prop=(dfa b i ct)

for c in 0 1 2 3; do
    solve=$dir/${prop[$c]}-solve.data
    run=$dir/${prop[$c]}-run.data
    sort -n $solve -o $solve
    sort -n $run -o $run
    count=1
    for line in `cat $solve`; do
        echo "($line, $count)" >> $solve-temp
        cp $solve-temp $solve
	count=$(($count+1))
    done
    count=1
    for line in `cat $run`; do
        echo "($line, $count)" >> $run-temp
        cp $run-temp $run
	count=$(($count+1))
    done        
done








REG="reg.log"
TUP_SPEED="tup_speed.log"
TUP_MEM="tup_mem.log"
CT="ct.log"

RAW="out.log"
RUNTIME="out-runtime.log"
OUT=$(echo $f | cut -d '.' -f1).data

touch $dir/$REG
touch $dir/$TUP_MEM
touch $dir/$TUP_SPEED
touch $dir/$CT

rm $dir/$REG
rm $dir/$TUP_MEM
rm $dir/$TUP_SPEED
rm $dir/$CT

cat $dir/out.log | grep "reg" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n >$dir/$REG
cat $dir/out.log | grep "tup_mem" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$TUP_MEM
cat $dir/out.log | grep "tup_speed" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$TUP_SPEED
cat $dir/out.log | grep "ct" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $dir/$CT

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

dirname `find $dir -name "ct.data"` | parallel --gnu -P 8 --progress nice ./generate-graph.sh {}

