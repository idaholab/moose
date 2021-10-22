# DGKernels System

# DGKernels System

## Overview

DGKernels are the kernels defined on internal sides.
DGKernels are typically for elemental variables, i.e. variables that allows solutions to be discontinous aross element sides.
DGKernels along with normal kernels allow the definition of weak forms rised from discontinous finite element methods (DFEM).
DGKernels can be block restricted for calculatons with DFEM on subdomains.
Internal sides are visited once during residual or Jacobian evaluations by MOOSE.
DGKernels handle two pieces of residual, marked as `Element` and `Neighbor`, on an internal side and corresponding four pieces of Jacobian, marked as `ElementElement`, `ElementNeighbor`, `NeighborElement` and `NeighborNeighbor`.
The normals on internal sides are pointing towards neighboring element from the current element.
Typically DGKernels are irrelevant with the normal direction.
When there are mesh refinement, MOOSE visits all the active internal sides, meaning that if there is a hanging node for an internal side, MOOSE visit the child internal sides.
DGKernels can make use of the material properties defined on both Element and Neighbor sides.
The DGKernel with interior penalty (IP) method for diffusion equations can be found at [DGDiffusion.md].
The DGKernel with upwinding scheme for hyperbolic equations can be found at [DGConvection.md].

## Extension for Hybrid Finite Element Methods

DGKernels are extended to support hybrid finite element methods (HFEM) [!citep](RT-HFEM).

Considering Poisson's equation of the form

\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h && \quad \in \partial \Omega_N, \\
  \alpha u + \frac{\partial u}{\partial n} &= c && \quad \in \partial \Omega_R,
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N \cup \partial \Omega_R$ is its boundary.
$\alpha$ is a given function on $\Omega_R$, which has typical constant value of $1/2$.

The weak form with HFEM for this PDE is to find a triple $(u, \lambda, \lambda_D)$, in discontinuous function spaces on $\Omega$, all internal sides $\Gamma$ and $\partial \Omega_D$ respectively, such that, $\forall (u^\ast, \lambda^\ast, \lambda^\ast_D)$ in the same function spaces,

\begin{equation}
\begin{aligned}
 \left( \nabla u^\ast, \nabla u \right)_\Omega + \left( \lambda^\ast, [ u ] \right)_\Gamma + \left( [ u^\ast ], \lambda \right)_\Gamma
 +\left( u^\ast, \lambda_D \right)_{\partial \Omega_D} + \left( \lambda_D^\ast, u - g \right)_{\partial \Omega_D}
 -\left( u^\ast, h \right)_{\partial \Omega_N}
 +\left( u^\ast, \alpha u - c \right)_{\partial \Omega_R}
 -\left( u^\ast, f \right)_\Omega = 0.
\end{aligned}
\end{equation}

$[u]$ represents the jump of $u$ on an internal side.
It is noted that the orientation of normals on internal sides does not affect the solution of $u$ but flips the sign of $\lambda$.
$\lambda$ and $\lambda_D$ are also known as Lagrange multipliers for weakly imposing the continuity of $u$ across internal sides on $\Gamma$ and imposing the Dirichlet boundary condition at $\Omega_D$.
They resemble the current ($-\frac{\partial u}{\partial n}$) and converge to the current when discretization error gets smaller and smaller with mesh refinement.
HFEM has explicit local conservation, which can be seen if we substitute a test function of $u^\ast$ with constant value of one element of interest and zero elsewhere.
The local conservation is evaluated with *Lagrange multiplier* $\lambda$ and the source function $f$ for an element inside of the domain $\Omega$.

This weak form requires a *compatibility condition* to have a unique solution [!citep](RT-HFEM).
Typically we satisfy this condition by letting the order of the shape function for $u$ two order higher (including two) than the order for $\lambda$.

An alternative way of imposing the Robin boundary condition ($\alpha u + \frac{\partial u}{\partial n} = c$) is to replace $\left( u^\ast, \alpha u - c \right)_{\partial \Omega_R}$ with

\begin{equation}
\begin{aligned}
\left( u^\ast, \lambda_R \right)_{\partial \Omega_R} + \left( \lambda_R^\ast, u - u_R \right)_{\partial \Omega_R} + \left( u_R^\ast, \alpha u_R - c - \lambda_R \right)_{\partial \Omega_R},
\end{aligned}
\end{equation}

with Lagrange multiplier $\lambda_R$ and the projected solution $u_R$ on $\partial \Omega_R$ and their corresponding test functions $\lambda_R^\ast$ and $u_R^\ast$.

A variable for the Lagrange multiplier defined on all interior sides $\Gamma$ can be coupled in DGKernels with a lower-dimensional mesh derived from the main mesh for $\Gamma$.
With this extension, DGKernels can handle three pieces of residual, marked as `Element`, `Neighbor` and `Lower`, on an internal side and corresponding nine pieces of Jacobian, marked as `ElementElement`, `ElementNeighbor`, `NeighborElement`, `NeighborNeighbor`, `PrimaryLower`, `SecondaryLower`, `LowerPrimary`, `LowerSecondary` and `LowerLower`.
Similarly, a variable for the Lagrange multiplier on boundary $\Omega_D$ can be coupled in integrated boundary conditions.

The DGKernal for $\left( \lambda^\ast, [ u ] \right)_\Gamma + \left( [ u^\ast ], \lambda \right)_\Gamma$ can be found at [HFEMDiffusion.md].
The boundary condition for $\left( u^\ast, \lambda_D \right)_{\partial \Omega_D} + \left( \lambda_D^\ast, u - g \right)_{\partial \Omega_D}$ can be found at [HFEMDirichletBC.md] with the generalization of $g$ being either a fixed value or a variable defined on boundary.
With this generalization, [HFEMDirichletBC.md] can be used along with kernels defined on the Robin boundary for $\left( u_R^\ast, \alpha u_R - c - \lambda_R \right)_{\partial \Omega_R}$ for the alternative way of imposing the Robin boundary condition above.

!alert warning
MOOSE does not support mesh adaptation with HFEM currently.

## Example Input File Syntax

DGKernels are added through `DGKernels` input syntax.

!syntax list /DGKernels objects=True actions=False subsystems=False

!syntax list /DGKernels objects=False actions=False subsystems=True

!syntax list /DGKernels objects=False actions=True subsystems=False
