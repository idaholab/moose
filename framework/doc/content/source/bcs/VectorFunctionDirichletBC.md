# VectorFunctionDirichletBC

!syntax description /BCs/VectorFunctionDirichletBC

## Description

`VectorFunctionDirichletBC` is a generalization of [`VectorDirichletBC`](/VectorDirichletBC.md) which
imposes possibly temporally- and spatially-dependent values for the Lagrange
vector components through
MOOSE [`Function`](/Functions/index.md) objects on degrees of freedom
that fall on the boundary defined by the `boundary` parameter. That is, for a
PDE of the form

\begin{equation}
\begin{aligned}
  -\nabla^2 \vec{u} &= \vec{f} && \quad \in \Omega \\
  \vec{u} &= \vec{g(t,\vec{x})} && \quad \in \partial \Omega_D \\
  \frac{\partial \vec{u}}{\partial n} &= \vec{h(t,\vec{x})} && \quad \in \partial \Omega_N
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary.
A `VectorFunctionDirichletBC` object can be used to impose the
condition (2) if the function is well-defined for $\vec{x} \in
\partial \Omega_D$. In this case, the `function_x`, `function_y`, and `function_z` parameters correspond to a
set of MOOSE `Function` objects which represent the mathematical function
$\vec{g(t,\vec{x})}$, and the user must define one or more sidesets
corresponding to the boundary subset $\partial \Omega_D$ via the
`boundary` parameter.

!syntax parameters /BCs/VectorFunctionDirichletBC

!syntax inputs /BCs/VectorFunctionDirichletBC

!syntax children /BCs/VectorFunctionDirichletBC
