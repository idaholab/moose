# FunctorNeumannBC

!syntax description /BCs/FunctorNeumannBC

## Description

`FunctorNeumannBC` is a generalization of [`ADNeumannBC`](/ADNeumannBC.md) which is used
for imposing flux boundary conditions on systems of partial
differential equations (PDEs) where the flux is represented by a
spatially- and temporally-varying MOOSE [`Function`](/Functions/index.md).  That is, for a
PDE of the form
\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g(t,\vec{x}) && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h(t,\vec{x}) && \quad \in \partial \Omega_N,
\end{aligned}
\end{equation}
where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary, a
`FunctorNeumannBC` object can be used to impose the third condition if the
function is well-defined for all relevant times and $\vec{x} \in
\partial \Omega_N$. In this case, the parameters [!param](/BCs/FunctorNeumannBC/functor)
and [!param](/BCs/FunctorNeumannBC/coefficient) are [functors](Functors/index.md),
whose product represents the mathematical function $h(t,\vec{x})$
(or $-h(t,\vec{x})$ if [!param](/BCs/FunctorNeumannBC/flux_is_inward) is set to `false`),
and the user must define one or more sidesets corresponding to the boundary
subset $\partial \Omega_N$ via the [!param](/BCs/FunctorNeumannBC/boundary) parameter.

!syntax parameters /BCs/FunctorNeumannBC

!syntax inputs /BCs/FunctorNeumannBC

!syntax children /BCs/FunctorNeumannBC
