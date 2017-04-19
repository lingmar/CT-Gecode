% CSPLib Problem 033: Word Design for DNA Computing on Surfaces
% http://www.csplib.org/Problems/prob033/
% MiniZinc model by Mats Carlsson

include "globals.mzn";

int: N;

array[1..16, 1..3] of base: equal_rel =
[| 0,  0, 1,
 | 0,  1, 0,
 | 0, 10, 0,
 | 0, 11, 0,
 | 1,  0, 0,
 | 1,  1, 1,
 | 1, 10, 0,
 | 1, 11, 0,
 |10,  0, 0,
 |10,  1, 0,
 |10, 10, 1,
 |10, 11, 0,
 |11,  0, 0,
 |11,  1, 0,
 |11, 10, 0,
 |11, 11, 1,
 |];

array[1..16, 1..3] of base: compl_rel =
[| 0,  0, 0,
 | 0,  1, 1,
 | 0, 10, 0,
 | 0, 11, 0,
 | 1,  0, 1,
 | 1,  1, 0,
 | 1, 10, 0,
 | 1, 11, 0,
 |10,  0, 0,
 |10,  1, 0,
 |10, 10, 0,
 |10, 11, 1,
 |11,  0, 0,
 |11,  1, 0,
 |11, 10, 1,
 |11, 11, 0,
 |];

set of int: base = {0,1,10,11};

array[1..N, 1..8] of var base: x;

constraint
  forall(i in 1..N)(sum(j in 1..8)(x[i,j]) in 40..48);

constraint
  forall(i in 2..N)(lex_less([x[i-1,j] | j in 1..5],[x[i,j] | j in 1..5]));

constraint
  forall(r1 in 1..N, r2 in 1..N where r1<r2)(
    let {array[1..8] of var 0..1: b}
    in forall(j in 1..8)(table([x[r1,j],x[r2,j],b[j]], equal_rel)) /\
    sum(b) <= 4
  );

constraint
  forall(r1 in 1..N, r2 in 1..N where r1<=r2)(
    let {array[1..8] of var 0..1: b}
    in forall(j in 1..8)(table([x[r1,j],x[r2,9-j],b[j]], compl_rel)) /\
    sum(b) <= 4
  );

solve :: int_search([x[i,j] | i in 1..N, j in 1..8], input_order, indomain_min, complete)
  satisfy;

output
  [ basechar(fix(x[i,j])) ++ if j=8 then "\n" else "" endif | i in 1..N, j in 1..8 ];

string: basechar(base: b) =
  if b=0 then "A"
  else if b=1 then "T"
  else if b=10 then "C"
  else "G"
  endif endif endif;

