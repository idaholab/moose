# SWEHydrostaticCorrectionDGKernel

Adds Audusse-style hydrostatic correction on faces for the shallow-water
momentum equations to preserve lake-at-rest (eta = h + b = const) exactly when
used with hydrostatic reconstruction.

!syntax description /DGKernels/SWEHydrostaticCorrectionDGKernel

Key requirements and parameters:

- Acts only on momentum variables (`hu` or `hv`).
- `h`, `hu`, `hv`: Coupled conservative variables.
- `b_var` (required): Cell-constant bathymetry variable (MONOMIAL/CONSTANT) used for hydrostatic
  reconstruction at faces. Must be the same cell-centered quantity used by the flux kernel.

Example usage (together with the flux kernel):

```
[DGKernels]
  [corr_hu]
    type = SWEHydrostaticCorrectionDGKernel
    variable = hu
    h = h
    hu = hu
    hv = hv
    b_var = b_field
  []
[]
```

!syntax parameters /DGKernels/SWEHydrostaticCorrectionDGKernel

!syntax inputs /DGKernels/SWEHydrostaticCorrectionDGKernel

!syntax children /DGKernels/SWEHydrostaticCorrectionDGKernel
