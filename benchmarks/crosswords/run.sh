#!/bin/bash

#./run_gecode.sh
#./run_compact_table.sh

while getopts ":v:" opt; do
  case $opt in
      :)
      VERBOSE=1
      ;;
  esac
done

echo "n & runtime_g & fail_g & nprops_g & runtime_c & fail_c & nprops_c"

# Compare solutions, assuming delimiter "-" between solution print and statistics
for i in {0..9}
do
    # Solution files
    solfile_g="solutions/gecode_$i.txt"
    solfile_c="solutions/compact-table_$i.txt"
    
    solution_g=$(cat $solfile_g | cut -d% -f1)
    solution_c=$(cat $solfile_c | cut -d% -f1)

    runtime_g=$(cat $solfile_g | grep "runtime" | grep -o "[0-9]*.[0-9][0-9][0-9]" | head -n 1)
    runtime_c=$(cat $solfile_c | grep "runtime" | grep -o "[0-9]*.[0-9][0-9][0-9]" | head -n 1)
    
    fail_g=$(cat $solfile_g | grep "failures" | grep -Eo "[0-9]{1,}")
    fail_c=$(cat $solfile_c | grep "failures" | grep -Eo "[0-9]{1,}")

    nprops_g=$(cat $solfile_g | grep "propagations" | grep -Eo "[0-9]{1,}")
    nprops_c=$(cat $solfile_c | grep "propagations" | grep -Eo "[0-9]{1,}")

    nsols_g=$(cat $solfile_g | grep "solutions" | grep -Eo "[0-9]{1,}")
    nsols_c=$(cat $solfile_c | grep "solutions" | grep -Eo "[0-9]{1,}")

    if [ $VERBOSE == 1 ]; then 
        if [ "$solution_c" != "$solution_g" ]; then
            echo "[WARN]: solutions differ for instance $i"
            diff <(echo $solution_g) <(echo $solution_c)
        fi
        if [ "$fail_g" != "$fail_c" ]; then
            echo "[WARN]: number of failures differ for instance $i"
            diff <(echo $fail_g) <(echo $fail_c)
        fi
        # if [ $nprops_g != $nprops_c ]; then
        #     echo "[WARN]: number of propagations differ for instance $i"
        #     diff <(echo $nprops_g) <(echo $nprops_c)
        # fi
        if [ "$nsols_g" != "$nsols_c" ]; then
            echo "[WARN]: number of solutions differ for instance $i"
            diff <(echo $nsols_g) <(echo $nsols_c)
        fi
    fi
    
    echo "$i & $runtime_g & $fail_g & $nprops_g & $runtime_c & $fail_c & $nprops_c \\\\"
    
done
