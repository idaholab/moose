# SWERDGReconstruction

Provides face-extrapolated values for `(h, hu, hv)` using linear reconstruction
and optional multi-D slope limiting. If no limiter is provided, passes cell averages.

!syntax description /Materials/SWERDGReconstruction

Key parameters:

- `h`, `hu`, `hv`: coupled variables (cell-averaged).
- `slope_limiting`: optional slope limiting UserObject (e.g., `SlopeLimitingBarthJespersen`).

!syntax parameters /Materials/SWERDGReconstruction

Notes

- For second-order MUSCL reconstruction, provide a slope limiting UserObject via `slope_limiting`.
  For 1D comparisons, you can use `SlopeLimitingOneDSWE` (minmod/MC/superbee) which returns
  limited x-slopes for `(h, hu, hv)`. Example:

```
[UserObjects]
  [limiter]
    type = SlopeLimitingOneDSWE
    h = h
    hu = hu
    hv = hv
    scheme = mc
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

!syntax inputs /Materials/SWERDGReconstruction

!syntax children /Materials/SWERDGReconstruction
