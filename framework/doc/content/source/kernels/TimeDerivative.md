# TimeDerivative

## Description

The `TimeDerivative` kernel implements a simple time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\frac{\partial u}{\partial t}}_{\textrm{TimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where the second term on the left hand side corresponds to the strong forms of
other kernels. The corresponding `TimeDerivative` weak form using inner-product notation is

\begin{equation}
R_i(u_h) = (\psi_i, \frac{\partial u_h}{\partial t}) \quad \forall \psi_i,
\end{equation}
where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The Jacobian is given by

\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = (\psi_i, a\phi_j).
\end{equation}
where $a$ is referred to as `du_dot_du` in MOOSE syntax. More information about time kernels can be
found on the Kernels description [page](syntax/Kernels/index.md).

## Example Syntax

Time derivative terms are ubiquitous in any transient simulation. The kernel
block for a transient advection-diffusion-reaction problem that demonstrates the
`TimeDerivative` syntax is shown below:

!listing test/tests/kernels/adv_diff_reaction/adv_diff_reaction_transient_test.i block=Kernels

!syntax parameters /Kernels/TimeDerivative

!syntax inputs /Kernels/TimeDerivative

!syntax children /Kernels/TimeDerivative
