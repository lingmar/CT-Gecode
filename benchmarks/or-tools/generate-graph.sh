#!/bin/bash

T=$1

REG=$T/reg.data
TUP_SPEED=$T/tup_speed.data
TUP_MEM=$T/tup_mem.data
CT=$T/ct.data

GRAPH=$ct/report/$1.tex


echo "\begin{axis}[
    xmode=log,
    ymin=0,ymax=`cat ct-ot.data | wc -l`,
    xmin=0.1, xmax=1000000,
    every axis plot/.style={thin},
    xlabel={timeout limit (ms)},
    ylabel={\# solved},
    legend pos=south east
    % table/create on use/cumulative distribution/.style={
    %   create col/expr={\pgfmathaccuma + \thisrow{f(x)}}   
    % }
    ]
    \addplot 
    [mark=triangle*,
    mark size=1.5,
    mark options={solid},
    green] 
    coordinates {
    $(cat $REG)
    };

    \addplot 
    [blue,
    mark=*,
    mark size=1.5,
    mark options={solid}]
    coordinates {
    $(cat $TUP_MEM)
    };

    \addplot [brown!60!black,
    mark options={fill=brown!40},
    mark=otimes*,
    mark size=1.5]
    coordinates {
    $(cat $TUP_SPEED)
    };

    \addplot 
    [red,
    mark size=1.5,
    mark=square*]
    coordinates {
    $(cat $CT)
    };
    \legend{DFA,B,I,CT}
  \end{axis}" > or-tools-plot.tex
