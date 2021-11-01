# ConstantRate

!syntax description /NodalKernels/ConstantRate

This ODE is solved at every node.
The [!param](/NodalKernels/ConstantRate/rate) parameter is controllable, so the [Control system](syntax/Controls/index.md) may be leveraged to dynamically control the rate during the simulation.

A more flexible alternative to controlling the rate with Controls is to use a [UserForcingFunctionNodalKernel.md] which has a rate that depends on space and time based on a [Function](syntax/Functions/index.md).

## Example input syntax

In this input file, the variable `lower` is the solution to the ordinary differential equation:

!equation
\dfrac{d lower}{d t} = -1

which is solved at every node on the block `lower`, which is a lower
dimensional subset of the square mesh. The constant rate term, $-1$ is
added using a `ConstantRate` nodal kernel.

!listing test/tests/bcs/ad_coupled_lower_value/test.i block=NodalKernels

!syntax parameters /NodalKernels/ConstantRate

!syntax inputs /NodalKernels/ConstantRate

!syntax children /NodalKernels/ConstantRate
