# NEML2StressDivergence

!if! function=hasCapability('neml2')

For a batch of material points, this object calculates the residual at each
point in the form $\phi^\alpha_{,j}\sigma_{ij}$, where
$\alpha=1,\ldots,n$ and $n$ is the number of displacement variables.

The kernel then assembles the integrated residual into the global residual vector.

The stress may be either a symmetric small/Cauchy stress in Mandel notation
(SR2) or a full, generally non-symmetric stress such as the first
Piola-Kirchhoff stress (R2) from a total-Lagrangian model. See
[NEML2DeformationGradient.md]. The weak form is identical for both, and the
reference-configuration quadrature makes the PK1 contraction the
total-Lagrangian residual.

## Limitations

- The current weak form is Cartesian-only; hoop and metric terms for
  axisymmetric or spherical coordinates are not present. For axisymmetric (RZ)
  problems, use [NEML2StressDivergenceRZ.md].
- This object assembles residuals only; no Jacobian contributions are produced.
  This system currently targets only explicit solves.
- Requires 1-3 displacement variables and uses the test functions from those variables' FE spaces.
- Pair this with matching, block-restricted `NEML2Assembly` and
  `NEML2FEInterpolation` objects if the mesh has mixed element types or orders.

## Syntax

!syntax parameters /UserObjects/NEML2StressDivergence

## Example input files

!syntax inputs /UserObjects/NEML2StressDivergence

!if-end!

!else

!include neml2/neml2_warning.md
