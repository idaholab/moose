# SWECharacteristicOutflowExactBoundaryFlux

Characteristic-based outflow boundary flux for the shallow-water equations (SWE).

- Projects the state onto the boundary normal and uses 1D SWE Riemann invariants.
- Subcritical outflow (|un| < c):
  - Preserves the outgoing interior characteristic R+ = un + 2 c.
  - Sets the incoming characteristic R− from a user-provided downstream target
    (target_depth `h_t` and target normal velocity `un_t`).
  - Builds a ghost state (h_R, un_R) and computes a Rusanov flux (full physical flux with
    pressure) between the interior and ghost.
- Supercritical outflow (|un| ≥ c): pure outflow (extrapolates interior state).

!syntax description /UserObjects/SWECharacteristicOutflowExactBoundaryFlux

## Parameters

- `gravity`: gravitational acceleration `g` (default 9.81).
- `dry_depth`: small depth threshold to avoid division by zero (default 1e-6).
- `target_depth`: downstream depth `h_t` used to set the incoming characteristic in
  subcritical outflow. If negative, falls back to pure outflow.
- `target_un`: downstream normal velocity `un_t` used with `target_depth`.
- `pressure_weight`: scales the pressure term in the boundary momentum flux between 0 (no
  pressure, advective-only) and 1 (full pressure). Default 0. Using a small value can reduce
  boundary oscillations for strongly transient outflow.

!syntax parameters /UserObjects/SWECharacteristicOutflowExactBoundaryFlux

## Usage

This UserObject is used as a boundary flux within the `SWEFluxBC` wrapper. A typical outflow
setup on the right boundary might look like:

```
[UserObjects]
  [outlet]
    type = SWECharacteristicOutflowExactBoundaryFlux
    gravity = 9.81
    dry_depth = 1e-6
    target_depth = 0.1   # downstream target depth (m)
    target_un = 0.0      # downstream target normal velocity (m/s)
  []
[]

[BCs]
  [outflow_h]
    type = SWEFluxBC
    variable = h
    boundary = 'right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []
  [outflow_hu]
    type = SWEFluxBC
    variable = hu
    boundary = 'right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []
  [outflow_hv]
    type = SWEFluxBC
    variable = hv
    boundary = 'right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []
[]
```

!syntax inputs /UserObjects/SWECharacteristicOutflowExactBoundaryFlux

!syntax children /UserObjects/SWECharacteristicOutflowExactBoundaryFlux
