# FVAdvection

!syntax description /FVKernels/FVAdvection

The `FVAdvection` kernel implements an advection term given for the domain ($\Omega$) defined as

!equation
\underbrace{\vec{v} \cdot \nabla u} + \sum_{i=1}^n \beta_i = 0 \in \Omega,

where $u$ is the advected quantity, the `variable` for this kernel, $v$ is the constant advecting velocity,
the `velocity` parameter of this kernel, and the $\beta_i$ are the contribution to the residual
of other kernels.

This volumetric term is transformed using the divergence theorem into a surface integral, computed
as a sum over each face of the advective flux. This is preferred over computing a volumetric gradient
as conservative advection is naturally achieved.

!equation
\int_{element} \vec{v} \cdot \nabla u = \sum_{elem faces f} u_f \vec{v} \cdot n_f v area_f

The advected quantity is evaluated on the face using an `advected_interp(olation)_method`.
Two methods are available:

- `average` for a geometrically weighted average between the element and neighbor values

- `upwind` for a first order upwind scheme, which uses the value from the centroid of the
  element situated upwind of the face, using velocity as the wind


!alert note
This kernel leverages the automatic differentiation system, so the Jacobian is
computed at the same time as the residual and need not be defined separately.

## Boundary conditions for pure advection

Advection problems, with a constant advecting velocity, should have two types of boundary conditions: inflow and outflow.
The inflow boundary conditions may be specified as a constant boundary value with a [FVDirichletBC.md] (with caveats, see
documentation)

!listing fvkernels/mms/advective-outflow/advection-diffusion.i block=FVBCs

The outflow boundary conditions may be specified with a [FVConstantScalarOutflowBC.md].

!listing test/tests/fvkernels/fv_constant_scalar_advection/2D_constant_scalar_advection.i block=FVBCs

If no boundary conditions are specified, then there is a zero advective flux through the boundary, also
known a no-penetration boundary condition.

!alert note
The `FVAdvection` kernel may be executed on boundaries using the `force_boundary_execution`
and `boundaries_to_force` parameters, however this is somewhat situational / not for mainstream use.

## Example input syntax

In this example, a simple time-dependent advection problem is solved, with a constant advecting velocity of
`1 0.5 0`.

!listing test/tests/fvkernels/fv_constant_scalar_advection/2D_constant_scalar_advection.i block=FVKernels

!syntax parameters /FVKernels/FVAdvection

!syntax inputs /FVKernels/FVAdvection

!syntax children /FVKernels/FVAdvection
