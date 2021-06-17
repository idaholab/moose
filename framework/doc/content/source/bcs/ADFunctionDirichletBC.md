# ADFunctionDirichletBC

!syntax description /BCs/ADFunctionDirichletBC

## Description

`ADFunctionDirichletBC` is a generalization of [`DirichletBC`](/DirichletBC.md) which
imposes a possibly temporally- and spatially-dependent value defined
by a MOOSE [`Function`](/Functions/index.md) object on a particular set of degrees of freedom
(DOFs) defined by the `boundary` parameter. That is, for a
PDE of the form

\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g(t,\vec{x}) && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h(t,\vec{x}) && \quad \in \partial \Omega_N
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary,
a `ADFunctionDirichletBC` object can be used to impose the
condition (2) if the function is well-defined for $\vec{x} \in
\partial \Omega_D$. In this case, the `function` parameter corresponds to a
MOOSE `Function` object which represents the mathematical function
$g(t,\vec{x})$, and the user must define one or more sidesets
corresponding to the boundary subset $\partial \Omega_D$ via the
`boundary` parameter.

Note that `ADFunctionDirichletBC` computes its Jacobian using automatic differentiation.

## Preset boundary conditions

With the parameter `preset = true`, the value of the boundary condition is applied
before the solve begins. With `preset = false`, the boundary condition is
only enforced as the solve progresses. In most situations, presetting the boundary
condition is better.

## Example Input Syntax

!listing test/tests/bcs/ad_bcs/ad_bc.i block=BCs

!syntax parameters /BCs/ADFunctionDirichletBC

!syntax inputs /BCs/ADFunctionDirichletBC

!syntax children /BCs/ADFunctionDirichletBC
