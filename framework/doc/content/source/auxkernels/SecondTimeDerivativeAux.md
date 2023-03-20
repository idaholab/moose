# SecondTimeDerivativeAux

!syntax description /AuxKernels/SecondTimeDerivativeAux

The [!param](/AuxKernels/SecondTimeDerivativeAux/factor) multiplies the output of the second time derivative operation.
The time derivative of the [!param](/AuxKernels/SecondTimeDerivativeAux/factor) is **not** computed.

!alert warning
The `SecondTimeDerivativeAux` can only output a non-zero time derivative when the [time integrator](syntax/Executioner/TimeIntegrator/index.md) has implemented the second time derivative. Only [CentralDifference.md] and [NewmarkBeta.md] have as of early 2023, but contributions are welcome!

## Example input syntax

In this example, the `SecondTimeDerivativeAux` is used to output to auxiliary variables the second time derivatives
of variables during a time dependent diffusion problem.

!listing test/tests/auxkernels/time_derivative_second_aux/test.i block=AuxKernels AuxVariables

!syntax parameters /AuxKernels/SecondTimeDerivativeAux

!syntax inputs /AuxKernels/SecondTimeDerivativeAux

!syntax children /AuxKernels/SecondTimeDerivativeAux
