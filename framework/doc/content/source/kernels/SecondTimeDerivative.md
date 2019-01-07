# SecondTimeDerivative

## Description

The `SecondTimeDerivative` kernel implements a simple second order time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\frac{\partial ^2 u}{\partial t ^2}}_{\textrm{SecondTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where the second term on the left hand side corresponds to the strong forms of
other kernels. The corresponding `SecondTimeDerivative` weak form using inner-product notation is

\begin{equation}
R_i(u_h) = (\psi_i, \frac{\partial ^2 u_h}{\partial t ^2}) \quad \forall \psi_i,
\end{equation}
where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The Jacobian is given by

\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = (\psi_i, a\phi_j).
\end{equation}
where $a$ is referred to as `du_dotdot_du` in MOOSE syntax. More information about time kernels can be
found on the Kernels description [page](syntax/Kernels/index.md).

## Example Syntax

Second Time derivative terms are ubiquitous in any transient simulation.
FIXME: need some examples demonstrating transient-diffusion problems with second order time derivatives.
