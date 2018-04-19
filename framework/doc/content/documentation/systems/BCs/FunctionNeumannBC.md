# FunctionNeumannBC

!syntax description /BCs/FunctionNeumannBC

## Description

`FunctionNeumannBC` is a generalization of [`NeumannBC`](/NeumannBC.md) which is used
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
`FunctionNeumannBC` object can be used to impose condition (3) if the
function is well-defined for all relevant times and $\vec{x} \in
\partial \Omega_N$. In this case, the `function` parameter corresponds
to a MOOSE `Function` object which represents the mathematical
function $h(t,\vec{x})$, and the user must define one or more sidesets
corresponding to the boundary subset $\partial \Omega_N$ via the
`boundary` parameter.

## Example Input Syntax

!listing test/tests/controls/time_periods/bcs/bcs_integrated.i start=[./right2] end=[../] include-end=true

!syntax parameters /BCs/FunctionNeumannBC

!syntax inputs /BCs/FunctionNeumannBC

!syntax children /BCs/FunctionNeumannBC
