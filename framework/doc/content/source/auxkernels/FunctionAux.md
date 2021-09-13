# FunctionAux

!syntax description /AuxKernels/FunctionAux

The `FunctionAux` helps turn a field defined by a function into a variable. It is generally
helpful when the spatial and temporal dependence of a field is known ahead of time, and the kernels
and other objects needing that field expect a variable rather than a function.

## Example syntax

In this example, the `FunctionAux` is used to store the exact solution of the problem, known
and defined as the function `aux_exact_fn` as an auxiliary variable `aux_u`.

!listing test/tests/auxkernels/element_var/element_var_test.i block=AuxKernels

!syntax parameters /AuxKernels/FunctionAux

!syntax inputs /AuxKernels/FunctionAux

!syntax children /AuxKernels/FunctionAux
