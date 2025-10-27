# SWENumericalFluxHLL

Implements a 2D HLL/Rusanov numerical flux for the shallow-water equations with
approximate analytic Jacobians (smax treated constant).

Implements a 2D Rusanov/HLL numerical flux for the shallow-water equations.

- Inputs left/right conservative states `U=(h,hu,hv)` and face unit normal `n`.
- Optionally takes bathymetry via a fourth entry `b` when provided by the kernel, and
  applies hydrostatic reconstruction (Audusse et al.) to preserve lake-at-rest.
- Enforces a dry threshold `dry_depth` to zero momentum when `h < dry_depth`.
- Provides approximate analytic Jacobians (wave speed treated constant).

!syntax description /UserObjects/SWENumericalFluxHLL

Key parameters:

- `gravity`: gravitational acceleration `g`.
- `dry_depth`: threshold for dry state handling.

!syntax parameters /UserObjects/SWENumericalFluxHLL

!syntax inputs /UserObjects/SWENumericalFluxHLL

!syntax children /UserObjects/SWENumericalFluxHLL
