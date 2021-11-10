# UserForcingFunctionNodalKernel

!syntax description /NodalKernels/UserForcingFunctionNodalKernel

The forcing function is a space and time dependent [Function](syntax/Functions/index.md).

The contribution to the Jacobian from this nodal kernel is 0, as
the function is deemed to not have any dependence on nonlinear variables, as functions generally do not.

## Example input syntax

In this example, the nodal scalar variable `v`, which lives independently on each node, is the solution to the following equation:

!equation
\dfrac{d v}{dt} = \dfrac{1}{4} (16*t + t^4)

The time dependent term on the right is specified using a `UserForcingFunctionNodalKernel`.

!listing test/tests/nodalkernels/high_order_time_integration/high_order_time_integration.i block=NodalKernels

!syntax parameters /NodalKernels/UserForcingFunctionNodalKernel

!syntax inputs /NodalKernels/UserForcingFunctionNodalKernel

!syntax children /NodalKernels/UserForcingFunctionNodalKernel
