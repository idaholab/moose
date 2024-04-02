# (Vector)CoupledTimeDerivative / ADCoupledTimeDerivative

!syntax description /Kernels/CoupledTimeDerivative

## Description

The `CoupledTimeDerivative` kernel is very similar to the
[`TimeDerivative`](/TimeDerivative.md) kernel with the
exception that the time derivative operator is applied to a coupled variable $v$
instead of the variable $u$ to whom's residual the `CoupledTimeDerivative` kernel
contributes. Consequently, the strong form on the the domain $\Omega$ is

\begin{equation}
\underbrace{k\frac{\partial v}{\partial t}}_{\textrm{CoupledTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega
\label{strong}
\end{equation}
where the second term on the left hand side corresponds to the
strong forms of other kernels. The `CoupledTimeDerivative` weak form is then

\begin{equation}
R_i(v_h) = \bigg(\psi_i, k\frac{\partial v_h}{\partial t}\bigg) \quad \forall
\psi_i,
\label{weak}
\end{equation}
where the $\psi_i$ are test functions, $k$ is a constant scalar coefficient and
$v_h \in \mathcal{S}^h$ is a finite element discretization of $v$.

For the `ADCoupledTimeDerivative` kernel, the Jacobian contribution is computed
using forward mode automatic differentiation.
For `CoupledTimeDerivative`, the Jacobian contribution is equal to
\begin{equation}
\frac{\partial R_i(v_h)}{\partial v_j} = (\psi_i, ka_v\phi_j).
\end{equation}
where $a_v$ is a constant that depends on the time stepping scheme; $a_v$ is
denoted by `_dv_dot` in the `CoupledTimeDerivative` class.

The `VectorCoupledTimeDerivative` kernel has the exact same semantics as
`CoupledTimeDerivative` but works on vector variables.
In that case, $u$, $v$, $v_h$, $\psi_i$, and $\phi_j$ above are all to be
understood as vector-valued functions.

## Example Input File Syntax

`CoupledTimeDerivative` is used for example in the split Cahn Hilliard equations
for phase field calculations. The syntax is simple, taking its type
(`CoupledTimeDerivative`), the variable to which the residual in
`CoupledTimeDerivative` contributes to, and the coupled variable `v` that the time
derivative operator acts upon. Example syntax can be found in the kernel block
below:

!listing test/tests/kernels/coupled_time_derivative/coupled_time_derivative_test.i block=Kernels

The syntax for `VectorCoupledTimeDerivative` is identical:

!listing test/tests/kernels/coupled_time_derivative/vector_coupled_time_derivative_test.i block=Kernels

And so is the syntax for `ADCoupledTimeDerivative`:

!listing test/tests/kernels/coupled_time_derivative/ad_coupled_time_derivative_test.i block=Kernels

!syntax parameters /Kernels/CoupledTimeDerivative

!syntax inputs /Kernels/CoupledTimeDerivative

!syntax children /Kernels/CoupledTimeDerivative
