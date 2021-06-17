# FVNeumannBC

!syntax description /FVBCs/FVNeumannBC

## Overview

A `FVNeumannBC` may be used to specify a diffusive or an advective flux. For example,
to specify a flux boundary condition in the following diffusion problem,
a `FVNeumannBC` with a constant value of $g$ may be used.

\begin{equation}
\begin{aligned}
  -\nabla^2 u(\mathbf{M}) &= f(\mathbf{M}) && \forall \mathbf{M} \in \Omega & (1)\\
  \frac{\partial u}{\partial n}(\mathbf{M}) &= g && \forall \mathbf{M} \in \partial \Omega_N & (2) \\
  u(\mathbf{M}) &= 1 && \forall \mathbf{M} \in \partial \Omega_D & (3)
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary, and $\mathbf{M}$ is
a point on the domain or its boundary. In this case, a `FVNeumannBC` object is used to impose
the condition (2) on the subset of the boundary denoted by $\partial \Omega_D$. In this case, the
`value` field corresponds to the constant $g$, and the user must define one
or more sidesets corresponding to the boundary $\partial \Omega_D$ to pass to the `boundary` argument.
For this particular problem, an additional boundary condition, for example a
`FVDirichletBC` as in (3) would also be necessary to remove the nullspace.

Likewise, to specify an advective flux of constant value $g$ in a 1D advection
problem with an advective velocity $v$:

\begin{equation}
\begin{aligned}
  \frac{\partial u}{\partial t}(\mathbf{M}) + v \frac{\partial u}{\partial x}(\mathbf{M}) &= 0 && \forall \mathbf{M} \in \Omega & (1)\\
  v u(\mathbf{M}) &= g v && \forall \mathbf{M} \in \partial \Omega & (2)
\end{aligned}
\end{equation}

The advective flux, the `value` to specify to the boundary condition (2), is $g v$.


Modeling a multi-dimensional problem will require a `FVNeumannBC` per component.

!alert note
When using the Navier Stokes module, `FVNeumannBC` may not be available for use with velocity
and pressure, as additional information is required on either the gradient or direction of
these variables to model fully developed flow for example. Specific boundary conditions are
provided, see for example `INSFVOutletPressureBC`.

!syntax parameters /FVBCs/FVNeumannBC

!syntax inputs /FVBCs/FVNeumannBC

!syntax children /FVBCs/FVNeumannBC
