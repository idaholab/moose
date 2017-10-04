
# FunctionDirichletBC
!syntax description /BCs/FunctionDirichletBC

## Description
`FunctionDirichletBC` is a generalization of [`DirichletBC`](/framework/DirichletBC.md) which
imposes a possibly temporally- and spatially-dependent value defined
by a MOOSE [`Function`](/Functions/index.md) object on a particular set of degrees of freedom
(DOFs) defined by the `boundary` parameter. That is, for a
PDE of the form
$$
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g(t,\vec{x}) && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h(t,\vec{x}) && \quad \in \partial \Omega_N
\end{aligned}
$$
where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary,
a `FunctionDirichletBC` object can be used to impose the
condition (2) if the function is well-defined for $\vec{x} \in
\partial \Omega_D$. In this case, the `function` parameter corresponds to a
MOOSE `Function` object which represents the mathematical function
$g(t,\vec{x})$, and the user must define one or more sidesets
corresponding to the boundary subset $\partial \Omega_D$ via the
`boundary` parameter.

## Example Input Syntax
!listing test/tests/bcs/function_dirichlet_bc/function_dirichlet_bc_test.i block=BCs

!syntax parameters /BCs/FunctionDirichletBC

!syntax inputs /BCs/FunctionDirichletBC

!syntax children /BCs/FunctionDirichletBC
