# FunctorDirichletBC

!syntax description /BCs/FunctorDirichletBC

## Description

`FunctorDirichletBC` is a generalization of [`DirichletBC`](/DirichletBC.md) which
imposes a possibly temporally- and spatially-dependent, possibly coupled to other variables/functors,
value defined by a MOOSE [`Functor`](/Functors/index.md) object on a particular set of degrees of freedom
(DOFs) defined by the `boundary` parameter. That is, for a
PDE of the form

\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g(t,\vec{x}) && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= c(t,,\vec{x},...) h(t,\vec{x}, ...) && \quad \in \partial \Omega_N
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary,
a `FunctorDirichletBC` object can be used to impose the
condition (2) if the Functor is well-defined for $\vec{x} \in
\partial \Omega_D$. In this case, the [!param](/BCs/FunctorDirichletBC/functor)
parameter corresponds to a MOOSE [Functor](Functors/index.md) object and `h` in the equation
for the boundary condition. Similarly `c` corresponds to a functor
[!param](/BCs/FunctorDirichletBC/coefficient) parameter. Both the functor and the coefficient
can have arbitrary dependencies on other functors, which can include variables, functions,
postprocessors, etc.

The user must define one or more sidesets
corresponding to the boundary subset $\partial \Omega_D$ via the
`boundary` parameter and it is the user's responsibility to ensure
the functor and the coefficient are defined there.

## Preset boundary conditions

With the parameter `preset = true`, the value of the boundary condition is applied
before the solve begins. With `preset = false`, the boundary condition is
only enforced as the solve progresses. In most situations, presetting the boundary
condition is better.

## Example Input Syntax

!listing test/tests/bcs/functor_dirichlet_bc/test.i block=BCs

!syntax parameters /BCs/FunctorDirichletBC

!syntax inputs /BCs/FunctorDirichletBC

!syntax children /BCs/FunctorDirichletBC
