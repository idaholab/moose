# CoupledTimeDerivative

## Description

The `CoupledTimeDerivative` kernel is very similar to the
[`TimeDerivative`](systems/Kernels/framework/TimeDerivative.md) kernel with the
exception that the time derivative operator is applied to a coupled variable $v$ instead
of the variable $u$ to whom's residual the `CoupledTimeDerivative` kernel
contributes. Consequently, the strong and weak forms are $$\frac{\partial
v}{\partial t}$$ and $$(\psi_i, \frac{\partial v_h}{\partial t})$$
respectively. The Jacobian contribution is equal to $$(\psi_i, a_v\phi_j)$$
where $a_v$ is a constant that depends on the time stepping scheme; $a_v$ is
denoted by `_dv_dot` in the `CoupledTimeDerivative` class.

## Example Syntax

`CoupledTimeDerivative` is used for example in the split Cahn Hilliard equations
for phase field calculations. Its syntax is simple, taking its type
(`CoupledTimeDerivative`), the variable to whom's residual the
`CoupledTimeDerivative` contributes, and the coupled variable `v` that the time
derivative operator acts upon. Example syntax can be found in the kernel block
below:

!listing
 test/tests/kernels/coupled_time_derivative/coupled_time_derivative_test.i
 block=Kernels label=false

!syntax parameters /Kernels/CoupledTimeDerivative

!syntax inputs /Kernels/CoupledTimeDerivative

!syntax children /Kernels/CoupledTimeDerivative
