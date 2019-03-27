# ADCoupledTimeDerivative

## Description

The `ADCoupledTimeDerivative` kernel is very similar to the
[`TimeDerivative`](/TimeDerivative.md) kernel with the
exception that the time derivative operator is applied to a coupled variable $v$ instead
of the variable $u$ to whom's residual the `ADCoupledTimeDerivative` kernel
contributes. Consequently, the strong form on the the domain $\Omega$ is

\begin{equation}
\underbrace{\frac{\partial v}{\partial t}}_{\textrm{ADCoupledTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega
\label{strong}
\end{equation}
where the second term on the left hand side corresponds to the
strong forms of other kernels. The `ADCoupledTimeDerivative` weak form is then

\begin{equation}
R_i(u_h) = \bigg(\psi_i, \frac{\partial v_h}{\partial t}\bigg) \quad \forall
\psi_i,
\label{weak}
\end{equation}
where the $\psi_i$ are test functions and $u_h \in \mathcal{S}^h$ is the finite
element solution of the weak formulation.

The Jacobian contribution is computed using forward mode automatic
differentiation.

## Example Syntax

`ADCoupledTimeDerivative` is used for example in the split Cahn Hilliard equations
for phase field calculations. The syntax is simple, taking its type
(`ADCoupledTimeDerivative`), the variable to that the residual the
`ADCoupledTimeDerivative` contributes, and the coupled variable `v` that the time
derivative operator acts upon. Example syntax can be found in the kernel block
below:

!listing
test/tests/kernels/coupled_time_derivative/ad_coupled_time_derivative_test.i
block=Kernels label=false

!syntax parameters /Kernels/ADCoupledTimeDerivative<RESIDUAL>

!syntax inputs /Kernels/ADCoupledTimeDerivative<RESIDUAL>

!syntax children /Kernels/ADCoupledTimeDerivative<RESIDUAL>
