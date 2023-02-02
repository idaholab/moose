# TimeDerivativeAux

!syntax description /AuxKernels/TimeDerivativeAux

The [!param](/AuxKernels/TimeDerivativeAux/factor) multiplies the output of the time derivative operation.
The time derivative of the [!param](/AuxKernels/TimeDerivativeAux/factor) is **not** computed.

!alert warning
The `TimeDerivativeAux` is restricted to constructs for which the time derivative is computed by MOOSE. Time derivative
functions have not been implemented for all functor types.

## Example input syntax

In this example, the `TimeDerivativeAux` is used to output to auxiliary variables the time derivatives
of variables and functions during a time dependent diffusion problem.

!listing test/tests/auxkernels/time_derivative_aux/test.i block=AuxKernels AuxVariables Functions

!syntax parameters /AuxKernels/TimeDerivativeAux

!syntax inputs /AuxKernels/TimeDerivativeAux

!syntax children /AuxKernels/TimeDerivativeAux
