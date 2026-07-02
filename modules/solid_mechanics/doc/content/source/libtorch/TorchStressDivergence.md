# TorchStressDivergence

!if! function=hasCapability('neml2')

For a batch of material points, calculate the residual at each point in the form of $\phi^\alpha_{,j} \sigma_{ij}$, for $\alpha = 1..n$ where $n$ is the number of displacement variables. The stress is read from a NEML2 model output (6-component Mandel) and expanded to its full form internally.

The kernel then assembles the integrated residual into the global residual vector.

## Limitations

- The current weak form is Cartesian-only; hoop/metric terms for axisymmetric or spherical coordinates are not present.
- This object assembles residuals only; no Jacobian contributions are produced. This system currently targets only explicit solves.
- Requires 1-3 displacement variables and uses the test functions from those variables' FE spaces.
- Pair this with a matching, block-restricted `TorchAssembly`/`TorchFEInterpolation` if you have mixed element types/orders.

## Syntax

!syntax parameters /UserObjects/TorchStressDivergence

## Example input files

!syntax inputs /UserObjects/TorchStressDivergence

!if-end!

!else

!include neml2/neml2_warning.md
