# DirichletBC

!syntax description /BCs/DirichletBC

## Description

`DirichletBC` is the simplest type of `NodalBC`, and is used for
imposing so-called "essential" boundary conditions on systems of
partial differential equations (PDEs).  Such boundary conditions force
a particular set of degrees of freedom (DOFs) defined by the
`boundary` parameter to take on a single, controllable value. This
class is appropriate to use for PDEs of the form

\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h && \quad \in \partial \Omega_N,
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary. In
this case, a `DirichletBC` object is used to impose the condition (2)
on the subset of the boundary denoted by $\partial \Omega_D$. In this case, the
`value` corresponds to the constant $g$, and the user must define one
or more sidesets corresponding to the boundary subset $\partial \Omega_D$.

## Example Input Syntax

!listing test/tests/bcs/matched_value_bc/matched_value_bc_test.i start=[./right_v] end=[../] include-end=true

!syntax parameters /BCs/DirichletBC

!syntax inputs /BCs/DirichletBC

!syntax children /BCs/DirichletBC
