# NEML2StressDivergence

!if! function=hasCapability('neml2')

For a batch of material points, calculate the residual at each point in the form of $\phi^\alpha_{,j} \sigma_{ij}$, for $\alpha = 1..n$ where $n$ is the number of displacement variables.

The kernel then assembles the integrated residual into the global residual vector.

## Limitations

- The current weak form is Cartesian-only; hoop/metric terms for axisymmetric or spherical coordinates are not present.
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
