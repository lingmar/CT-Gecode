#!/bin/bash

T=$1

prop=(dfa b i ct)

DFA_R=$T/dfa-run.data
B_R=$T/b-run.data
I_R=$T/i-run.data
CT_R=$T/ct-run.data

DFA_S=$T/dfa-solve.data
B_S=$T/b-solve.data
I_S=$T/i-solve.data
CT_S=$T/ct-solve.data

GRAPH_S=$ct/report/$1-solve.tex
GRAPH_R=$ct/report/$1-run.tex

echo "\begin{axis}[
    xmode=log,
    ymin=0,ymax=`cat $CT_R | wc -l`,
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
    $(cat $DFA_R)
    };

    \addplot 
    [blue,
    mark=*,
    mark size=1.5,
    mark options={solid}]
    coordinates {
    $(cat $B_R)
    };

    \addplot [brown!60!black,
    mark options={fill=brown!40},
    mark=otimes*,
    mark size=1.5]
    coordinates {
    $(cat $I_R)
    };

    \addplot 
    [red,
    mark size=1.5,
    mark=square*]
    coordinates {
    $(cat $CT_R)
    };
    \legend{DFA,B,I,CT}
  \end{axis}" > $GRAPH_R

echo "\begin{axis}[
    xmode=log,
    ymin=0,ymax=`cat $CT_S | wc -l`,
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
    $(cat $DFA_S)
    };

    \addplot 
    [blue,
    mark=*,
    mark size=1.5,
    mark options={solid}]
    coordinates {
    $(cat $B_S)
    };

    \addplot [brown!60!black,
    mark options={fill=brown!40},
    mark=otimes*,
    mark size=1.5]
    coordinates {
    $(cat $I_S)
    };

    \addplot 
    [red,
    mark size=1.5,
    mark=square*]
    coordinates {
    $(cat $CT_S)
    };
    \legend{DFA,B,I,CT}
  \end{axis}" > $GRAPH_S
