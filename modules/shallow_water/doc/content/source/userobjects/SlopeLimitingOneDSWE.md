# SlopeLimitingOneDSWE

!syntax description /UserObjects/SlopeLimitingOneDSWE

1D TVD MUSCL slope limiter for shallow-water conservative variables `(h, hu, hv)` along the
streamwise (x) direction. It computes limited linear slopes using left/right neighbor cell
averages and returns a vector of `RealGradient` slopes for use by `SWERDGReconstruction`.

- Supports schemes: `none`, `minmod`, `mc` (monotonized central, default), and `superbee`.
- Only the x-component of the slope is nonzero. On elements with `n_sides != 2` the limiter
  returns zero slopes (first-order reconstruction).
- Intended for quick 1D comparisons of HLL vs HLLC with second-order MUSCL reconstruction.

!syntax parameters /UserObjects/SlopeLimitingOneDSWE

Parameters

- `h`, `hu`, `hv` (required): Coupled cell-constant variables for depth and momenta.
- `scheme` (optional): One of `none|minmod|mc|superbee` (default: `mc`).

!syntax inputs /UserObjects/SlopeLimitingOneDSWE

Example (1D dam-break with second-order MUSCL + limiter)

```
[UserObjects]
  [limiter]
    type = SlopeLimitingOneDSWE
    h = h
    hu = hu
    hv = hv
    scheme = mc
    # Recommended: freeze slopes during Newton for robust convergence
    execute_on = 'INITIAL TIMESTEP_BEGIN TIMESTEP_END'
  []
[]

[Materials]
  [recon]
    type = SWERDGReconstruction
    h = h
    hu = hu
    hv = hv
    slope_limiting = limiter
  []
[]
```

!syntax children /UserObjects/SlopeLimitingOneDSWE
