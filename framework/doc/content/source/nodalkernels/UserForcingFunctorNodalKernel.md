# UserForcingFunctorNodalKernel

!syntax description /NodalKernels/UserForcingFunctorNodalKernel

The forcing Functor is a [Functor](syntax/Functors/index.md).

## Example input syntax

In this example, the nodal scalar variable `v`, which lives independently on each node, is the solution to the following equation:

!equation
\dfrac{d v}{dt} = \dfrac{1}{4} (16*t + t^4)

The time dependent term on the right is specified using a `UserForcingFunctorNodalKernel`.

!listing test/tests/nodalkernels/high_order_time_integration/high_order_time_integration.i block=NodalKernels

!syntax parameters /NodalKernels/UserForcingFunctorNodalKernel

!syntax inputs /NodalKernels/UserForcingFunctorNodalKernel

!syntax children /NodalKernels/UserForcingFunctorNodalKernel
