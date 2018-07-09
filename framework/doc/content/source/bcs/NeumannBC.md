# NeumannBC

!syntax description /BCs/NeumannBC

## Description

`NeumannBC` is the simplest type of `IntegratedBC`, and is used for
imposing flux boundary conditions on systems of partial differential
equations (PDEs). This class is appropriate to use for PDEs of the
form
\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h && \quad \in \partial \Omega_N,
\end{aligned}
\end{equation}
where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary. In
this case, a `NeumannBC` object is used to impose the condition (3) on
the subset of the boundary denoted by $\partial \Omega_N$. The `value`
parameter corresponds to the constant $h$, and the user must define
one or more sidesets corresponding to the boundary subset $\partial
\Omega_N$.  The normal derivative notation is $\frac{\partial
u}{\partial n} \equiv \nabla u \cdot \hat{n}$, where $\hat{n}$ is the
outward unit normal to $\partial \Omega_N$.

## Example Input Syntax

!listing test/tests/bcs/1d_neumann/1d_neumann.i start=[./right] end=[../] include-end=true

!syntax parameters /BCs/NeumannBC

!syntax inputs /BCs/NeumannBC

!syntax children /BCs/NeumannBC
