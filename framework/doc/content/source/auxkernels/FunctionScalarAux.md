# FunctionScalarAux

!syntax description /AuxScalarKernels/FunctionScalarAux

The functions will be evaluated at the current time and the system origin (0,0,0).

## Example syntax

In this example, this auxiliary scalar kernel is being used to convert a function output
into a scalar variable, to verify the correctness of another system (parsed functions).

!listing test/tests/functions/parsed/scalar.i block=AuxScalarKernels

!syntax parameters /AuxScalarKernels/FunctionScalarAux

!syntax inputs /AuxScalarKernels/FunctionScalarAux

!syntax children /AuxScalarKernels/FunctionScalarAux
