# FVIntegralValueConstraint

This object implements the residuals that enforce the constraint

!equation
\int_{\Omega} \phi = \int_{\Omega} \phi_0

using a Lagrange multiplier approach. E.g. this object enforces the constraint
that the average value of $\phi$ match $\phi_0$.

The detailed description of the derivation for the corresponding finite element
constraint can be found at
[scalar_constraint_kernel](https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf). The
finite volume version can be obtained by simply substituting $1$ for
$\varphi$. Note that $\int \phi_0 = V_0$.

!alert warning
The contribution to the diagonal of the system of this kernel is null, which introduces a saddle
point. Make sure to use a `NONZERO` shift in your preconditioner.

## Example input syntax

In this example, the average value of the variable `v` is set using a `FVIntegralValueConstraint`.
In combination with a single Dirichlet boundary condition, this makes the numerical problem accept a
single numerical solution, and be well-posed.

!listing test/tests/fvkernels/constraints/integral.i block=Variables FVKernels

!syntax parameters /FVKernels/FVIntegralValueConstraint

!syntax inputs /FVKernels/FVIntegralValueConstraint

!syntax children /FVKernels/FVIntegralValueConstraint
