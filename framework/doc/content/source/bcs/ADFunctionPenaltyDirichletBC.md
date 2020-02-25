# ADFunctionPenaltyDirichletBC

!syntax description /BCs/ADFunctionPenaltyDirichletBC

## Description

`ADFunctionPenaltyDirichletBC` is a generalization of [`ADPenaltyDirichletBC`](/ADPenaltyDirichletBC.md) which
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
a `ADFunctionPenaltyDirichletBC` object can be used to impose the
condition (2) if the function is well-defined for $\vec{x} \in
\partial \Omega_D$. In this case, the `function` parameter corresponds to a
MOOSE `Function` object which represents the mathematical function
$g(t,\vec{x})$, and the user must define one or more sidesets
corresponding to the boundary subset $\partial \Omega_D$ via the
`boundary` parameter.

Instead of imposing the Dirichlet condition directly on the basis by replacing the
equations associated with those degrees of freedom (DOFs) by the auxiliary equation
$u-g(t,\vec{x})=0$, the `ADFunctionPenaltyDirichletBC` is based on the variational statement:
find $u \in H^1(\Omega)$ such that
\begin{equation}
  \label{weakform}
  \int_{\Omega} \left( \nabla u \cdot \nabla v - fv \right) \,\text{d}x
  -\int_{\partial \Omega_N} hv \,\text{d}s
  +\int_{\partial \Omega_D} \frac{1}{\epsilon} (u-g(t,\vec{x}))v \,\text{d}s = 0
\end{equation}
holds for every $v \in H^1(\Omega)$. In [weakform], $\epsilon
\ll 1$ is a user-selected parameter which must be taken small enough
to ensure that $u \approx g(t,\vec{x})$ on $\partial \Omega_D$. The
user-selectable class parameter `penalty` corresponds to
$\frac{1}{\epsilon}$, and must be chosen large enough to ensure
good agreement with the Dirichlet data, but not so large that the
resulting Jacobian becomes ill-conditioned, resulting in failed solves
and overall accuracy losses.

Benefits of the penalty-based approach include simplified Dirichlet
boundary condition enforcement for non-Lagrange finite element bases,
maintaining the symmetry (if any) of the original problem, and
avoiding the need to zero out contributions from other rows in a
special post-assembly step. Integrating by parts "in reverse"
from [weakform], one obtains

\begin{equation}
  \label{weakform2}
  \int_{\Omega} \left( -\nabla^2 u  - f \right) v \,\text{d}x
  +\int_{\partial \Omega_N} \left( \frac{\partial u}{\partial n} - h(t,\vec{x}) \right) v \,\text{d}s
  +\int_{\partial \Omega_D} \left[ \frac{\partial u}{\partial n} + \frac{1}{\epsilon} (u-g(t,\vec{x})) \right] v \,\text{d}s = 0
\end{equation}

We therefore recover a "perturbed" version of the original problem with the flux
boundary condition

\begin{equation}
  \frac{\partial u}{\partial n} = -\frac{1}{\epsilon} (u-g(t,\vec{x})) \, \in \partial \Omega_D
\end{equation}

replacing the original Dirichlet boundary condition. It has been shown
[!cite](juntunen2009nitsche) that in order for the solution to this perturbed
problem to converge to the solution of the original problem in the
limit as $\epsilon \rightarrow 0$, the penalty parameter must depend
on the mesh size, and that as we refine the mesh, the problem becomes
increasingly ill-conditioned.  A related method for imposing Dirichlet boundary
conditions, known as Nitsche's method [!cite](juntunen2009nitsche), does not
suffer from the same ill-conditioning issues, and is slated for inclusion
in MOOSE some time in the future.


## Example Input Syntax

!listing test/tests/bcs/ad_penalty_dirichlet_bc/function_penalty_dirichlet_bc_test.i block=BCs

!syntax parameters /BCs/ADFunctionPenaltyDirichletBC

!syntax inputs /BCs/ADFunctionPenaltyDirichletBC

!syntax children /BCs/ADFunctionPenaltyDirichletBC
