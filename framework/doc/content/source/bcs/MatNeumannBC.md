# MatNeumannBC

!syntax description /BCs/MatNeumannBC

## Description

`MatNeumannBC` is a generalization of [`NeumannBC`](/NeumannBC.md) which is used
for imposing flux boundary conditions on systems of partial
differential equations (PDEs) where the flux is represented by the product
of a constant and a MOOSE Material [`Material`](/Materials/index.md).  That is, for a
PDE of the form
\begin{equation}
\begin{aligned}
  -\nabla \cdot C (\nabla u) &= f && \quad \in \Omega \\
  u &= g(t,\vec{x}) && \quad \in \partial \Omega_D \\
  C \frac{\partial u}{\partial n} &= hM(t,\vec{x}) && \quad \in \partial \Omega_N,
\end{aligned}
\end{equation}
where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary, a
`MatNeumannBC` object can be used to impose condition (3) if the
material is well-defined for all relevant times and $\vec{x} \in
\partial \Omega_N$. In this case, the `boundary_material` parameter corresponds
to a MOOSE `Material` object, and the user must define one or more sidesets
corresponding to the boundary subset $\partial \Omega_N$ via the
`boundary` parameter.

## Example Input Syntax

!listing test/tests/bcs/mat_neumann_bc/mat_neumann.i block=BCs/top

!syntax parameters /BCs/MatNeumannBC

!syntax inputs /BCs/MatNeumannBC

!syntax children /BCs/MatNeumannBC
