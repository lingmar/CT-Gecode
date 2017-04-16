#!/bin/bash

export TABLE_PROPAGATOR=$1

file=$2

${GECODE_PATH}/bin/fzn-gecode -s $file #> "solutions/compact-table_$i.txt"

