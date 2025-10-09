# SWEFVFluxDGKernel

Assembles side fluxes for the 2D shallow-water equations in a cell-centered FV (rDG) fashion.

- Expects face-extrapolated `MaterialProperty` values `h`, `hu`, `hv` from a reconstruction
  material and passes an optional `b` (bathymetry) property to the numerical flux.
- Uses a `BoundaryFluxBase`-style numerical flux UserObject (e.g., [`SWENumericalFluxHLL`](SWENumericalFluxHLL.md)).
- Builds full 3×3 Element/Neighbor Jacobian blocks for Newton solves.

!syntax description /DGKernels/SWEFVFluxDGKernel

Key parameters:

- `h`, `hu`, `hv`: coupled variables.
- `numerical_flux`: internal side flux userobject.

Key requirements and parameters:

- `h`, `hu`, `hv`: Coupled conservative variables.
- `numerical_flux`: Numerical flux UserObject (e.g., [`SWENumericalFluxHLL`](SWENumericalFluxHLL.md) or [`SWENumericalFluxHLLC`](SWENumericalFluxHLLC.md)).
- `b_var` (required): Cell-constant bathymetry variable (MONOMIAL/CONSTANT) coupled as a primary
  variable. The value must be piecewise constant per cell for exact lake-at-rest balance.

Example wiring for `b_var` using an Aux variable:

```
[AuxVariables]
  [b_field]
    family = MONOMIAL
    order  = CONSTANT
  []
[]

[AuxKernels]
  [b_out]
    type     = FunctionAux
    variable = b_field
    function = bump        # or flat, etc.
  []
[]

[DGKernels]
  [flux_h]
    type = SWEFVFluxDGKernel
    variable = h
    h = h
    hu = hu
    hv = hv
    b_var = b_field
    numerical_flux = flux
  []
[]
```

!syntax parameters /DGKernels/SWEFVFluxDGKernel

!syntax inputs /DGKernels/SWEFVFluxDGKernel

!syntax children /DGKernels/SWEFVFluxDGKernel
