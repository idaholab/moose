# SWEFrictionSource

Manning friction source term for SWE momentum equations:

For `hu`: Sx = − g n^2 u |u| / h^(1/3), and for `hv`: Sy analogous.

!syntax description /Kernels/SWEFrictionSource

Key parameters:

- `manning_n`: Manning roughness coefficient.
- `gravity`, `dry_depth`, `speed_eps` for robustness.

!syntax parameters /Kernels/SWEFrictionSource

!syntax inputs /Kernels/SWEFrictionSource

!syntax children /Kernels/SWEFrictionSource
