# ConstraintGradCollector

!syntax description /Reporters/ConstraintGradCollector

## Overview

`ConstraintGradCollector` provides the ability to use constraints with multiple
parameters.



## Graphical Representation

Given 3 set parameters values with {i,j,k} values,  equality constraints
{a,b,c}, and defining $a_n$ as the derivative of the constraint with respect to
the $n^{th}$ parameter the `ConstraintGradCollector` will construct total
vectors in the correct form below. Currently the assumption is that constraints
are specific to a single of parameter group. If the constraint is on all the
parameters then this reporter is not needed because it would already be in the
form below.

$\begin{bmatrix}
a_1 & 0 & 0 \\
a_2 & 0 & 0 \\
\vdots & \vdots & \vdots \\
a_i & 0 & 0 \\
0 & b_1 & 0 \\
0 & b_2 & 0 \\
\vdots & \vdots & \vdots \\
0 & b_j & 0 \\
0 & 0 & c_1 \\
0 & 0 & c_2 \\
\vdots & \vdots & \vdots \\
0 & 0 & c_k \\
\end{bmatrix}$




!syntax parameters /Reporters/ConstraintGradCollector

!syntax inputs /Reporters/ConstraintGradCollector

!syntax children /Reporters/ConstraintGradCollector
