# (Vector)TimeDerivative

!syntax description /Kernels/TimeDerivative

## Overview

The `TimeDerivative` kernel implements a simple time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{k\frac{\partial u}{\partial t}}_{\textrm{TimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where the second term on the left hand side corresponds to the strong forms of
other kernels. The corresponding `TimeDerivative` weak form using inner-product notation is

\begin{equation}
R_i(u_h) = (\psi_i, k\frac{\partial u_h}{\partial t}) \quad \forall \psi_i,
\end{equation}
where $u_h$ is the approximate solution, $k$ is a constant scalar coefficient
and $\psi_i$ is a finite element test function.

The Jacobian is given by

\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = (\psi_i, ka\phi_j).
\end{equation}
where $a$ is referred to as `du_dot_du` in MOOSE syntax.

The `VectorTimeDerivative` kernel has the exact same semantics but works on vector variables.
In that case, $u$, $u_h$, $\psi_i$, and $\phi_j$ above are all to be
understood as vector-valued functions.

More information about time kernels can be
found on the Kernels description [page](syntax/Kernels/index.md).

## Example Input File Syntax

Time derivative terms are ubiquitous in any transient simulation. The kernel
block for a transient advection-diffusion-reaction problem that demonstrates the
`TimeDerivative` syntax is shown below:

!listing test/tests/kernels/adv_diff_reaction/adv_diff_reaction_transient_test.i block=Kernels

The kernel block for a transient diffusion problem that demonstrates the
`VectorTimeDerivative` syntax is shown below:

!listing test/tests/kernels/transient_vector_diffusion/transient_vector_diffusion.i block=Kernels

!syntax parameters /Kernels/TimeDerivative

!syntax inputs /Kernels/TimeDerivative

!syntax children /Kernels/TimeDerivative
