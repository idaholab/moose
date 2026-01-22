# SWEOpenBoundaryRiemannFlux

!syntax description /UserObjects/SWEOpenBoundaryRiemannFlux

Implements an open/outflow boundary for the shallow-water equations using a ghost-state Riemann
approach. At each boundary quadrature point, a ghost state is built by:

- Preserving the outgoing characteristic from the interior, and
- Setting the incoming characteristic from a far-field target (stage or dry/vacuum),

in the boundary-normal direction. The flux is then computed by reusing the same HLLC/HLL numerical
flux user object as on interior faces, including hydrostatic reconstruction with bathymetry when a
`b` entry is provided.

This preserves lake-at-rest over variable bathymetry at the outlet and avoids ad-hoc pressure
blending or outflow gating. Works for 1D and 2D with oblique waves (via normal/tangential split).

!syntax parameters /UserObjects/SWEOpenBoundaryRiemannFlux

!syntax inputs /UserObjects/SWEOpenBoundaryRiemannFlux

!syntax children /UserObjects/SWEOpenBoundaryRiemannFlux

## Example

```
[UserObjects]
  [hllc]
    type = SWENumericalFluxHLLC
    gravity = 9.81
    dry_depth = 1e-6
  []
  [open_outlet]
    type = SWEOpenBoundaryRiemannFlux
    numerical_flux = hllc
    gravity = 9.81
    dry_depth = 1e-6
    farfield_mode = stage
    eta_infty = 1.0
    u_n_infty = 0
    u_t_infty = 0
    allow_backflow = true
  []
[]

[BCs]
  [outlet_h]
    type = SWEFluxBC
    boundary = 'right'
    variable = h
    h = h
    hu = hu
    hv = hv
    boundary_flux = open_outlet
    b_var = b_field   # optional, but recommended to preserve lake-at-rest
  []
  # Repeat for hu,hv ...
```

