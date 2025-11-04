# ArrayVarReductionAux

!syntax description /AuxKernels/ArrayVarReductionAux

# Description

This auxiliary kernel performs a reduction operation on the values of an array variable $g$ (indexed as $g_i$).
Reduction operations include max, min, sum, and average.
The operations sum and average can optionally use weights by setting the [!param](/AuxKernels/ArrayVarReductionAux/weights) parameter.
These perform the following operations:

sum without weights:

!equation
f = \sum\limits_{i=1}^I g_i.

sum with weights:

!equation
f = \sum\limits_{i=1}^I g_i w_i.

average without weights:

!equation
f = \frac{1}{I}\sum\limits_{i=1}^I g_i.

average with weights:

!equation
f = \frac{1}{\sum\limits_{i=1}^I w_i}\sum\limits_{i=1}^I g_i w_i.

min:

!equation
f = \min\limits_{i=1,I} g_i.

max:

!equation
f = \max\limits_{i=1,I} g_i.

!syntax parameters /AuxKernels/ArrayVarReductionAux

!syntax inputs /AuxKernels/ArrayVarReductionAux

!syntax children /AuxKernels/ArrayVarReductionAux
