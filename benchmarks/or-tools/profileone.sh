FZN=$1
OUT=$(echo $PERF | rev | cut -f 2- -d '.' | rev)
CSV=$(echo $PERF | rev | cut -f 2- -d '.' | rev).profile.csv

# Warm up first
export TABLE_PROPAGATOR=foo
${GECODE_PATH}/bin/fzn-gecode -time 1000000 -s $FZN > /dev/null
# Perform profiling
for prop in "gecode-regular" "gecode-tupleset-mem" "gecode-tupleset-speed" "compact-table"; do
    export TABLE_PROPAGATOR=$prop
    PERF=$OUT.$prop.data
    perf record -o $PERF
    res=$prop
    for entry in "propagate" "advise" "copy"; do
        res="$res;"${perf report -i $PERF | grep -E $entry | grep -Eo "[0-9]+.[0-9]+\%" | cut -d "%" -f1}
    done
    echo $res | sed 's/;$//' >> $CSV
done
