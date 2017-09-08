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
for phase field calculations. An example of its syntax and use is given in
the `Kernel` block below:

!listing modules/phase_field/tutorials/spinodal_decomposition/s1_testmodel.i
 block=Kernels label=false

 The `CoupledTimeDerivative` term corresponds to the first term in equation 6 on
 the phase field equation
 [page](modules/phase_field/Phase_Field_Equations.md).

!syntax parameters /Kernels/CoupledTimeDerivative

!syntax inputs /Kernels/CoupledTimeDerivative

!syntax children /Kernels/CoupledTimeDerivative
