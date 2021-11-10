# ODETimeDerivative

!syntax description /ScalarKernels/ODETimeDerivative

A scalar variable can be set to the solution of an ordinary differential equation (ODE), as specified in [the Scalar Kernels syntax page](syntax/ScalarKernels/index.md). This kernel adds a time derivative term. The time integration scheme will be shared with the other non-linear variables. To use a different time integrating scheme, the `ODETimeDerivative` scalar kernel should be replaced with a custom implementation.

## Example input syntax

In this example, the scalar variables `x` and `y` are the solutions to the coupled ODE problem:

!equation
\dfrac{d x}{d t} -3*x - 2*y = 0

!equation
\dfrac{d y}{d t} -4*x - y = 0

The time derivative terms are added for each variable using two `ODETimeDerivative` scalar kernels.

!listing test/tests/kernels/ode/parsedode_sys_impl_test.i block=ScalarKernels

!syntax parameters /ScalarKernels/ODETimeDerivative

!syntax inputs /ScalarKernels/ODETimeDerivative

!syntax children /ScalarKernels/ODETimeDerivative
