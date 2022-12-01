# ArrayVarReductionAux

!syntax description /AuxKernels/ArrayVarReductionAux

# Description

This auxiliary kernel performs a reduction operation on the values of an array variable $g$ (indexed as $g_i$).
Reduction operations include max, min, sum, and average.
These perform the following operations:

sum:

!equation
f = \sum\limits_{i=1}^I g_i.

average:

!equation
f = \frac{1}{I}\sum\limits_{i=1}^I g_i.

min:

!equation
f = \min\limits_{i=1,I} g_i.

max:

!equation
f = \max\limits_{i=1,I} g_i.

!syntax parameters /AuxKernels/ArrayVarReductionAux

!syntax inputs /AuxKernels/ArrayVarReductionAux

!syntax children /AuxKernels/ArrayVarReductionAux
