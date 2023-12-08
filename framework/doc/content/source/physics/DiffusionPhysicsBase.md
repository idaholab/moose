# DiffusionPhysicsBase

This is a base class for the derived [Physics](syntax/Physics/index.md) actions setting up objects
to solve the diffusion equation with a particular discretization.

The diffusion equation solved is:

!equation
\dfrac{\partial u}{\partial t} + \nabla \cdot D \nabla u(\vec{x}) - f(\vec{x}) = 0

with Dirichlet boundary conditions:

!equation
u(\vec{x}) = g

and / or Neumann boundary conditions:

!equation
\dfrac{\partial u}{\partial n} = h

over the boundaries specified by the [!param](/Physics/DiffusionFV/dirichlet_boundaries) and
[!param](/Physics/DiffusionFV/neumann_boundaries) parameters respectively.

The values set at the Dirichlet boundary conditions, $g$, and Neumann boundary conditions, $h$,
are set by the [!param](/Physics/DiffusionFV/boundary_values) and
[!param](/Physics/DiffusionFV/boundary_fluxes) respectively.
