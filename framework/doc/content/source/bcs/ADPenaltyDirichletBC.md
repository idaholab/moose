# ADPenaltyDirichletBC

!syntax description /BCs/ADPenaltyDirichletBC

## Description

`ADPenaltyDirichletBC` is a `ADIntegratedBC` used for enforcing Dirichlet boundary conditions
which differs from the [`ADDirichletBC`](/ADDirichletBC.md) class in the way in which it handles the enforcement.
It is appropriate for partial differential equations (PDEs) in the form

\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h && \quad \in \partial \Omega_N
\end{aligned}
\end{equation}

Instead of imposing the Dirichlet condition directly on the basis by replacing the
equations associated with those degrees of freedom (DOFs) by the auxiliary equation
$u-g=0$, the `ADPenaltyDirichletBC` is based on the variational statement:
find $u \in H^1(\Omega)$ such that
\begin{equation}
  \label{weakform}
  \int_{\Omega} \left( \nabla u \cdot \nabla v - fv \right) \,\text{d}x
  -\int_{\partial \Omega_N} hv \,\text{d}s
  +\int_{\partial \Omega_D} \frac{1}{\epsilon} (u-g)v \,\text{d}s = 0
\end{equation}
holds for every $v \in H^1(\Omega)$. In [weakform], $\epsilon
\ll 1$ is a user-selected parameter which must be taken small enough
to ensure that $u \approx g$ on $\partial \Omega_D$. The
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
  +\int_{\partial \Omega_N} \left( \frac{\partial u}{\partial n} - h \right) v \,\text{d}s
  +\int_{\partial \Omega_D} \left[ \frac{\partial u}{\partial n} + \frac{1}{\epsilon} (u-g) \right] v \,\text{d}s = 0
\end{equation}

We therefore recover a "perturbed" version of the original problem with the flux
boundary condition

\begin{equation}
  \frac{\partial u}{\partial n} = -\frac{1}{\epsilon} (u-g) \, \in \partial \Omega_D
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



!bibtex bibliography



## Example Input Syntax

!listing test/tests/bcs/ad_penalty_dirichlet_bc/penalty_dirichlet_bc_test.i block=BCs

!syntax parameters /BCs/ADPenaltyDirichletBC

!syntax inputs /BCs/ADPenaltyDirichletBC

!syntax children /BCs/ADPenaltyDirichletBC
