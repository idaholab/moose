# CoupledODETimeDerivative

!syntax description /ScalarKernels/CoupledODETimeDerivative

The `CoupledODETimeDerivative` is similar to the `ODETimeDerivative` scalar kernel, except that the derivative acts on a different variable than the ODE is solved for.

## Example input syntax

In this example, the scalar variables `f` and `f_times_mult` are the solutions to the coupled ODE problem:

!equation
\dfrac{d f\_times\_mult}{d t} -1 = 0

!equation
f\_times\_mult -f * g = 0

The time derivative term for the first equation is added using a `CoupledODETimeDerivative` scalar kernel, as the first equation is set to have `f` as its nonlinear variable, even though `f` does not appear in it!

!listing test/tests/kernels/ode/coupled_ode_td.i block=ScalarKernels

!syntax parameters /ScalarKernels/CoupledODETimeDerivative

!syntax inputs /ScalarKernels/CoupledODETimeDerivative

!syntax children /ScalarKernels/CoupledODETimeDerivative
