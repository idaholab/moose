# ArrayCoupledTimeDerivative

## Description

The `ArrayCoupledTimeDerivative` kernel is very similar to the
[`CoupledTimeDerivative`](/CoupledTimeDerivative.md) kernel with the
exception that it works for array variables rather than scalars.
The strong form on the domain $\Omega$ is

\begin{equation}
\underbrace{\frac{\partial v}{\partial t}}_{\textrm{ArrayCoupledTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega
\label{strong}
\end{equation}
where the second term on the left hand side corresponds to the
strong forms of other kernels. The `ArrayCoupledTimeDerivative` weak form is then

\begin{equation}
R_i(u_h) = \bigg(\psi_i, \frac{\partial v_h}{\partial t}\bigg) \quad \forall
\psi_i,
\label{weak}
\end{equation}
where the $\psi_i$ are test functions and $u_h \in \mathcal{S}^h$ is the finite
element solution of the weak formulation.

The Jacobian contribution is equal to
\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = (\psi_i, a_v\phi_j).
\end{equation}
where $a_v$ is a constant that depends on the time stepping scheme; $a_v$ is
denoted by `_dv_dot` in the `ArrayCoupledTimeDerivative` class.

## Example Syntax

The syntax is simple, taking its type (`ArrayCoupledTimeDerivative`), the variable
to that the residual the `ArrayCoupledTimeDerivative` contributes, and the coupled
variable `v` that the time derivative operator acts upon. Example syntax can be
found in the kernel block below:

!listing test/tests/kernels/array_coupled_time_derivative/test_jacobian.i block=Kernels

!syntax parameters /Kernels/ArrayCoupledTimeDerivative

!syntax inputs /Kernels/ArrayCoupledTimeDerivative

!syntax children /Kernels/ArrayCoupledTimeDerivative
