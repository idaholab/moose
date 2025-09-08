# TidalGravityAux

Computes a scalar gravity field `g(t,x)` for shallow-water models with an optional
simple global tide correction from the Sun and Moon, projected onto the local vertical.

!syntax description /AuxKernels/TidalGravityAux

The AuxKernel writes `g` for a MONOMIAL/CONSTANT AuxVariable so values are
element-wise constant and consistent on faces, matching how bathymetry is handled.

Model

- Assumes a spherical domain centered at the origin; local vertical is the outward
  unit vector `n_r = x/|x|`.
- Effective gravity vector is `g_vec = -g0 * n_r + a_tide(t)` where `a_tide(t)` is a uniform
  vector from simplified circular Sun/Moon orbits. The scalar gravity used by SWE terms is
  `g = -g_vec · n_r` which removes lateral components.

Parameters

- `g0`: base gravity magnitude [m/s^2], default `9.81`.
- `enable_tides`: toggle Sun/Moon tide correction, default `true`.
- `simulation_start_epoch`: absolute UTC for simulation `t=0` in seconds since 1970-01-01.
- `earth_radius`: Earth mean radius [m], default `6.371e6`.
- `mu_sun`, `sun_distance`, `sun_period`: Sun GM [m^3/s^2], mean distance [m], orbit period [s].
- `mu_moon`, `moon_distance`, `moon_period`: Moon GM [m^3/s^2], mean distance [m], sidereal period [s].

Example

```
[AuxVariables]
  [g_field]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [g_out]
    type = TidalGravityAux
    variable = g_field
    g0 = 9.81
    enable_tides = true
    simulation_start_epoch = 1735689600 # 2025-01-01T00:00:00Z
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]
```

!syntax parameters /AuxKernels/TidalGravityAux

!syntax inputs /AuxKernels/TidalGravityAux

!syntax children /AuxKernels/TidalGravityAux
