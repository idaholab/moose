# VectorTimeDerivative

!syntax description /Kernels/VectorTimeDerivative

## Overview

The `VectorTimeDerivative` kernel implements a simple time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\frac{\partial \vec{u}}{\partial t}}_{\textrm{VectorTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where the second term on the left hand side corresponds to the strong forms of
other kernels. The corresponding `VectorTimeDerivative` weak form using inner-product notation is

\begin{equation}
R_i(\vec{u}_h) = (\vec{\psi}_i, \frac{\partial \vec{u_h}}{\partial t}) \quad \forall \vec{\psi}_i,
\end{equation}
where $\vec{u}_h$ is the approximate solution and $\vec{\psi}_i$ is a finite element test function.

The Jacobian is given by

\begin{equation}
\frac{\partial R_i(\vec{u}_h)}{\partial u_j} = (\vec{\psi}_i, a\vec{\phi}_j).
\end{equation}
where $a$ is referred to as `du_dot_du` in MOOSE syntax. More information about time kernels can be
found on the Kernels description [page](syntax/Kernels/index.md).

## Example Input File Syntax

Time derivative terms are ubiquitous in any transient simulation. The kernel
block for a transient diffusion problem that demonstrates the
`VectorTimeDerivative` syntax is shown below:

!listing test/tests/kernels/transient_vector_diffusion/transient_vector_diffusion.i block=Kernels

!syntax parameters /Kernels/VectorTimeDerivative

!syntax inputs /Kernels/VectorTimeDerivative

!syntax children /Kernels/VectorTimeDerivative
