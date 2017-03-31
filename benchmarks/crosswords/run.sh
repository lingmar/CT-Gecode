#!/bin/bash

#./run_gecode.sh
#./run_compact_table.sh

VERBOSE=0
PLOT=0
TIMEOUTMSG="=====UNKNOWN====="

while getopts ":v:" opt; do
  case $opt in
      :)
      VERBOSE=1
      ;;
  esac
done

while getopts ":p:" opt; do
  case $opt in
      :)
      PLOT=1
      ;;
  esac
done

#echo "n & runtime_g & fail_g & nprops_g & runtime_c & fail_c & nprops_c"

if [ PLOT == 1 ]; then
    echo "ct ge"
fi

# Compare solutions, assuming delimiter "-" between solution print and statistics
for i in {0..71}
do
    # Solution files
    solfile_g="solutions/gecode_"$i".txt"
    solfile_c="solutions/compact-table_"$i".txt"

    firstline_g=$(head -n 1 $solfile_g)
    firstline_c=$(head -n 1 $solfile_c)

    if [ "$firstline_c" == $TIMEOUTMSG ]; then
        runtime_c="\Timeout"
    else
        runtime_c=$(cat $solfile_c | grep "runtime" | grep -o "[0-9]*.[0-9][0-9][0-9]" | head -n 1)
    fi

    if [ "$firstline_g" == $TIMEOUTMSG ]; then
        runtime_g="\Timeout"
    else
        runtime_g=$(cat $solfile_g | grep "runtime" | grep -o "[0-9]*.[0-9][0-9][0-9]" | head -n 1)
    fi
   
    # Find the solution
    #solution_g=$(cat $solfile_g | cut -d% -f1)
    #solution_c=$(cat $solfile_c | cut -d% -f1)
    
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
    
    if [ $PLOT == 1 ]; then
	echo "$i & $runtime_g & $fail_g & $nprops_g & $runtime_c & $fail_c & $nprops_c \\\\"
    else
	echo "$runtime_c $runtime_g"	
    fi
done
