# SWENumericalFluxHLLC

Implements an HLLC numerical flux for the 2D shallow-water equations using a
1D HLLC Riemann solver along the face normal with hydrostatic reconstruction.

- Supports optional bathymetry input (fourth entry `b`) for lake-at-rest preservation.
- Tangential velocity is advected unchanged across the contact wave.
- Provides approximate analytic Jacobians (based on physical flux + Rusanov contribution).

Parameters

- `use_pvrs` (bool, default `true`): Use PVRS star-depth with q-scaled acoustic speeds for sharper
  shocks. If `false`, use Einfeldt/Davis wave-speed bounds.
- `degeneracy_eps` (Real, default `1e-8`): Small epsilon used to clamp `S*` and detect degeneracy
  or non-positive star depths; triggers HLLE fallback locally when violated.
- `blend_alpha` (Real in $[0, 0.5]$, default `0.0`): Optional compression-based blend to HLLE in
  strong compression, only when `S_L < 0 < S_R`. Set to 0 to disable.

!syntax description /UserObjects/SWENumericalFluxHLLC

!syntax parameters /UserObjects/SWENumericalFluxHLLC

!syntax inputs /UserObjects/SWENumericalFluxHLLC

!syntax children /UserObjects/SWENumericalFluxHLLC
