# TimeDerivativeNodalKernel

!syntax description /NodalKernels/TimeDerivativeNodalKernel

This kernel will use the same time stepping scheme as specified for
other nonlinear variables in the [Executioner](syntax/Executioner/index.md).

## Example input syntax

In this input file, the variable `lower` is the solution to the ordinary differential equation:

!equation
\dfrac{d lower}{d t} = -1

which is solved at every node on the block `lower`, which is a lower
dimensional subset of the square mesh. The time derivative term is
added using a `TimeDerivativeNodalKernel`.

!listing test/tests/bcs/ad_coupled_lower_value/test.i block=NodalKernels

!syntax parameters /NodalKernels/TimeDerivativeNodalKernel

!syntax inputs /NodalKernels/TimeDerivativeNodalKernel

!syntax children /NodalKernels/TimeDerivativeNodalKernel
