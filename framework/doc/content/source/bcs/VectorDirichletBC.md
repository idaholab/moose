# VectorDirichletBC

!syntax description /BCs/VectorDirichletBC

## Description

`VectorDirichletBC` is the extension of [`DirichletBC`](bcs/DirichletBC) to
Lagrange vector variables and is used for
imposing so-called "essential" boundary conditions on systems of
partial differential equations (PDEs).  Such boundary conditions force
a particular set of degrees of freedom (DOFs) defined by the
`boundary` parameter to take on controllable values. This
class is appropriate to use for PDEs of the form
\begin{equation}
\begin{aligned}
  -\nabla^2 \vec{u} &= \vec{f} && \quad \in \Omega \\
  \vec{u} &= \vec{g} && \quad \in \partial \Omega_D \\
  \frac{\partial \vec{u}}{\partial n} &= \vec{h} && \quad \in \partial \Omega_N
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary. In
this case, a `VectorDirichletBC` object is used to impose the condition (2)
on the subset of the boundary denoted by $\partial \Omega_D$. In this case, the
`values` correspond to the constant components of $\vec{g}$, and the user must define one
or more sidesets corresponding to the boundary subset $\partial \Omega_D$.

!syntax parameters /BCs/VectorDirichletBC

!syntax inputs /BCs/VectorDirichletBC

!syntax children /BCs/VectorDirichletBC
