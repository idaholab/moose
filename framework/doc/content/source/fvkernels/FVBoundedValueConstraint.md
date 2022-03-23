# FVBoundedValueConstraint

This object implements the residuals that enforce the constraint

!equation
\phi > \phi_0 \in \Omega

or

!equation
\phi < \phi_0 \in \Omega

using a Lagrange multiplier approach. E.g. this object enforces the constraint
that the value of $\phi$ in the domain has to be above or below a certain value.

!alert warning
The contribution to the diagonal of the system of this kernel is null, which introduces a saddle
point. Make sure to use a `NONZERO` shift in your preconditioner.

## Example input syntax

In this example, the value of the variable `v` is forced to be positive using a `FVBoundedValueConstraint`.
This accelerates the convergence to the solution, as the initial condition is negative and the solution
is everywhere positive.

!listing test/tests/fvkernels/constraints/point_value.i block=Variables FVKernels

!syntax parameters /FVKernels/FVBoundedValueConstraint

!syntax inputs /FVKernels/FVBoundedValueConstraint

!syntax children /FVKernels/FVBoundedValueConstraint
