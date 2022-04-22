# TimeDerivativeAux

!syntax description /AuxKernels/TimeDerivativeAux

The [!param](/AuxKernels/TimeDerivativeAux/factor) multiplies the output of the time derivative operation.
The time derivative of the [!param](/AuxKernels/TimeDerivativeAux/factor) is **not** computed.

For AD input objects such as:

- variables
- AD functions
- AD functor material properties

an `ADTimeDerivativeAux` should be created. The AD derivatives are however discarded when filling the auxiliary variable.

!alert warning
The `TimeDerivativeAux` is restricted to constructs for which the time derivative is computed by MOOSE. Time derivative
functions have not been implemented for all functor types.

!alert warning
The time derivative of an auxiliary variable currently cannot be computed by an auxiliary kernel using the
coupled variable interface.

!alert warning
The time derivative of a finite volume variable currently cannot be computed by the coupled variable interface, so it
should be passed as the [!param](/AuxKernels/TimeDerivativeAux/functor) parameter and not the
[!param](/AuxKernels/TimeDerivativeAux/v) parameter.

## Example input syntax

In this example, the `TimeDerivativeAux` is used to output to auxiliary variables the time derivatives
of variables and functions during a time dependent diffusion problem.

!listing test/tests/auxkernels/time_derivative_aux/test.i block=AuxKernels AuxVariables Functions

!syntax parameters /AuxKernels/TimeDerivativeAux

!syntax inputs /AuxKernels/TimeDerivativeAux

!syntax children /AuxKernels/TimeDerivativeAux
