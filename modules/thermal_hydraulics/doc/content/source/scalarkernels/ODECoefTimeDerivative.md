# ODECoefTimeDerivative

!syntax description /ScalarKernels/ODECoefTimeDerivative

A scalar variable can be set to the solution of an ordinary differential equation (ODE), as specified in [the Scalar Kernels syntax page](syntax/ScalarKernels/index.md). This kernel adds a time derivative term. The time integration scheme will be shared with the other non-linear variables, as specified in the [TimeIntegrator](syntax/Executioner/TimeIntegrator/index.md).
To use a different time integrating scheme, the `ODECoefTimeDerivative` scalar kernel should be replaced with a custom implementation.

!syntax parameters /ScalarKernels/ODECoefTimeDerivative

!syntax inputs /ScalarKernels/ODECoefTimeDerivative

!syntax children /ScalarKernels/ODECoefTimeDerivative
