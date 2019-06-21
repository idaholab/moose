# DiffusionFluxBC

!syntax description /BCs/DiffusionFluxBC

## Description

`DiffusionFluxBC` is a `FluxBC` which is appropriate for use with the boundary terms arising from the
[`Diffusion`](/Diffusion.md) [`Kernel`](syntax/Kernels/index.md). `DiffusionFluxBC` does not
"enforce" a boundary condition per-se (see, e.g. [`DirichletBC`](/DirichletBC.md),
[`NeumannBC`](/NeumannBC.md), and related classes for that).  Instead, this class is responsible for
computing the residual (and Jacobian) contributions due to the boundary contribution arising from
integration by parts on the [`Diffusion`](/Diffusion.md) [`Kernel`](syntax/Kernels/index.md).

!alert note
The standard theory of elliptic operators requires the specification of boundary conditions on all
parts of the boundary, so "implicitly" computing a residual contribution in this manner (instead of
replacing it with the correct "data") falls outside of this theory. That said, there are instances
where such an approach gives reasonable results in practice, see, for example, the paper by
[!cite](griffiths1997noboundary).

As an example, consider the Poisson problem with mixed
boundary conditions:
\begin{equation}
\begin{aligned}
  -\nabla^2 u &= f && \quad \in \Omega \\
  u &= g && \quad \in \partial \Omega_D \\
  \frac{\partial u}{\partial n} &= h && \quad \in \partial \Omega_N,
\end{aligned}
\end{equation}
where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N \cup \partial \Omega_F$ is its boundary,
and no boundary conditions are specified on $\partial \Omega_F$.
The weak formulation of this problem is: find $u$ satisfying the Dirichlet
boundary conditions and such that
\begin{equation}
  \label{weakform}
  \int_{\Omega} \left( \nabla u \cdot \nabla v - fv \right) \,\text{d}x
  -\int_{\partial \Omega_N} hv \,\text{d}s
  -\underbrace{\int_{\partial \Omega_F} \frac{\partial u}{\partial n} v \,\text{d}s}_{\texttt{DiffusionFluxBC}} = 0
\end{equation}
holds for every
$v \in \mathcal{V} = \{v : v \in H^1(\Omega), v=0 \text{ on } \partial \Omega_D \}$,
i.e. test functions that vanish on the Dirichlet boundary. The `DiffusionFluxBC`
class would then be used to compute the last term in [weakform].

!bibtex bibliography


## Example Input Syntax

!listing test/tests/mortar/continuity-2d-conforming/equalgradient.i start=[./all] end=[../] include-end=true

!syntax parameters /BCs/DiffusionFluxBC

!syntax inputs /BCs/DiffusionFluxBC

!syntax children /BCs/DiffusionFluxBC
