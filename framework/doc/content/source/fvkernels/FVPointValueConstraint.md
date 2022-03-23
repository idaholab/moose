# FVPointValueConstraint

This object implements the residuals that enforce the constraint

!equation
\phi (point P) = \phi_0

using a Lagrange multiplier approach. E.g. this object enforces the constraint
that the value of $\phi$ in the element containing point $P$ match $\phi_0$.

!alert warning
The contribution to the diagonal of the system of this kernel is null, which introduces a saddle
point. Make sure to use a `NONZERO` shift in your preconditioner.

## Example input syntax

In this example, the value of the variable `v` at `0.2 0 0` is set using a `FVPointValueConstraint`.
In combination with a single Dirichlet boundary condition, this makes the numerical problem accept a
single numerical solution, and be well-posed.

!listing test/tests/fvkernels/constraints/point_value.i block=Variables FVKernels

!syntax parameters /FVKernels/FVPointValueConstraint

!syntax inputs /FVKernels/FVPointValueConstraint

!syntax children /FVKernels/FVPointValueConstraint
