# RateDirichletBC

!syntax description /BCs/RateDirichletBC

## Description

`RateDirichletBC` is a of [`DirichletBC`](/DirichletBC.md) in which
the value imposed on a particular set of degrees of freedom (DOFs)
increases at a user-defined rate. This corresponds to PDE of the form

\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  \delta u &= g \delta t && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h && \quad \in \partial \Omega_N,
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary, and
$\delta u$ is the increment of the variable $u$ between two instants
$t$ and $t + \delta t$. In this case, the `rate` parameter corresponds to
the constant $g$ and is a controllable value, and the user must define
one or more sidesets corresponding to the boundary subset
$\partial \Omega_D$ via the `boundary` parameter.

## Example Input Syntax

!listing test/tests/bcs/rate_bcs/bc_rate_dirichlet.i start=[./right_rate] end=[../] include-end=true

!syntax parameters /BCs/RateDirichletBC

!syntax inputs /BCs/RateDirichletBC

!syntax children /BCs/RateDirichletBC
