# AbaqusDLoadInterpolator

!syntax description /UserObjects/AbaqusDLoadInterpolator

## Description

`AbaqusDLoadInterpolator` precomputes per-element distributed load arrays at the beginning of each
timestep by interpolating between the begin/end `*Dload` maps provided by
[AbaqusUELStepUserObject](AbaqusUELStepUserObject.md). The resulting `JDLTYP` (face/type codes) and
`ADLMAG` (magnitudes) arrays are then consumed by
[AbaqusUELMeshUserElement](AbaqusUELMeshUserElement.md) and passed to the UEL plugin through the
standard `NDLOAD/JDLTYP/ADLMAG/MDLOAD` interface.

## Usage

- `step_user_object`: name of the [AbaqusUELStepUserObject](AbaqusUELStepUserObject.md) that holds
  the current step state and DLOAD definitions. Defaults to `step_uo`.

This user object executes on `INITIAL` and `TIMESTEP_BEGIN` and is intended to be referenced by
`AbaqusUELMeshUserElement` via its `dload_interpolator` parameter (default: `dload_uo`).

!syntax parameters /UserObjects/AbaqusDLoadInterpolator

!syntax inputs /UserObjects/AbaqusDLoadInterpolator

!syntax children /UserObjects/AbaqusDLoadInterpolator

