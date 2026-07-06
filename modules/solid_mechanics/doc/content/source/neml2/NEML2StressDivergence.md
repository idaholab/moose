# NEML2StressDivergence

!if! function=hasCapability('neml2')

For a batch of material points, calculate the residual at each point in the form of $\phi^\alpha_{,j} \sigma_{ij}$, for $\alpha = 1..n$ where $n$ is the number of displacement variables.

The kernel then assembles the integrated residual into the global residual vector.

The stress may be either a symmetric small/Cauchy stress in Mandel notation (SR2) or a full, generally non-symmetric stress such as the first Piola-Kirchhoff stress (R2) from a total-Lagrangian model (see [NEML2DeformationGradient.md]); the weak form is identical for both, and the reference-configuration quadrature makes the PK1 contraction exactly the total-Lagrangian residual.

## Limitations

- The current weak form is Cartesian-only; hoop/metric terms for axisymmetric or spherical coordinates are not present. For axisymmetric (RZ) problems, use [NEML2StressDivergenceRZ.md].
- This object assembles residuals only; no Jacobian contributions are produced. This system currently targets only explicit solves.
- Requires 1-3 displacement variables and uses the test functions from those variables' FE spaces.
- Pair this with a matching, block-restricted `NEML2Assembly`/`NEML2FEInterpolation` if you have mixed element types/orders.

## Syntax

!syntax parameters /UserObjects/NEML2StressDivergence

## Example input files

!syntax inputs /UserObjects/NEML2StressDivergence

!if-end!

!else

!include neml2/neml2_warning.md
