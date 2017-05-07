#!/bin/bash

scp liin3244@siegbahn.it.uu.se:/it/slask/student/liin3244/CT-Gecode/benchmarks/or-tools/*.data .

echo "\begin{tikzpicture}[scale=1.0]
  \begin{axis}[
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
    table {reg-ot.data};

    \addplot 
    [blue,
    mark=*,
    mark size=1.5,
    mark options={solid}]
    table {tup_mem-ot.data};

    \addplot [brown!60!black,
    mark options={fill=brown!40},
    mark=otimes*,
    mark size=1.5]
    table {tup_speed-ot.data};

    \addplot 
    [red,
    mark size=1.5,
    mark=square*]
    table {ct-ot.data};
    \legend{Reg,Tup\_mem,Tup\_speed,CT}
  \end{axis}
\end{tikzpicture}" > or-tools-plot.tex
