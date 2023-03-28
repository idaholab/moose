# FVBoundaryIntegralValueConstraint

This object implements the residuals that enforce the constraint

!equation
\int_{\partial \Omega} \phi = \int_{\partial \Omega} \phi_0

using a Lagrange multiplier approach. E.g. this object enforces the constraint that the average
value of $\phi$ match $\phi_0$ on the prescribed `boundary` (denoted by $\partial \Omega$ in the
equation above). This object is the boundary version of the volumetric object
[FVIntegralValueConstraint.md].

!alert warning
The contribution to the diagonal of the system of this kernel is null, which introduces a saddle
point. Make sure to use a `NONZERO` shift in your preconditioner.

## Example input syntax

In this example, the average value of the variable `v` is set on the `right`
boundary using a `FVBoundaryIntegralValueConstraint`.
In combination with a single Dirichlet boundary condition on the `left` boundary, this makes the numerical problem accept a
single numerical solution, and be well-posed.

!listing test/tests/fvkernels/fv_simple_diffusion/dirichlet-constrained-average-value.i block=FVBCs

!syntax parameters /FVBCs/FVBoundaryIntegralValueConstraint

!syntax inputs /FVBCs/FVBoundaryIntegralValueConstraint

!syntax children /FVBCs/FVBoundaryIntegralValueConstraint
