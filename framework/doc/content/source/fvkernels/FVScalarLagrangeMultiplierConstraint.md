# FVScalarLagrangeMultiplierConstraint

This object is a base class for implementing variable value constraints using Lagrange multipliers.

The detailed description of the derivation for the corresponding finite element
constraint can be found at
[scalar_constraint_kernel](https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf). The
finite volume version can be obtained by simply substituting $1$ for
$\varphi$.

!alert warning
The contribution to the diagonal of the system of this kernel is null, which introduces a saddle
point. Make sure to use a `NONZERO` shift in your preconditioner.
