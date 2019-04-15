# ADTimeDerivative

## Description

The `ADTimeDerivative` kernel implements a simple time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\frac{\partial u}{\partial t}}_{\textrm{ADTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where the second term on the left hand side corresponds to the strong forms of
other kernels. The corresponding `ADTimeDerivative` weak form using inner-product notation is

\begin{equation}
R_i(u_h) = (\psi_i, \frac{\partial u_h}{\partial t}) \quad \forall \psi_i,
\end{equation}
where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The Jacobian is computed through automatic differentiation. More information about time kernels can be
found on the Kernels description [page](syntax/Kernels/index.md).

## Example Syntax

Time derivative terms are ubiquitous in any transient simulation. The kernel
block for a transient diffusion problem that demonstrates the
`ADTimeDerivative` syntax is shown below:

!listing test/tests/kernels/ad_transient_diffusion/ad_transient_diffusion.i block=Kernels

!syntax parameters /Kernels/ADTimeDerivative

!syntax inputs /Kernels/ADTimeDerivative

!syntax children /Kernels/ADTimeDerivative
