# AbaqusUELMeshUserElement

!syntax description /UserObjects/AbaqusUELMeshUserElement

## Description

`AbaqusUELMeshUserElement` executes Abaqus UEL plugins on top of an
[AbaqusUELMesh](AbaqusUELMesh.md). It is similar in spirit to
[AbaqusUserElement](AbaqusUserElement.md) but designed for UEL meshes that represent user elements
via node elements and a custom connectivity map, enabling arbitrary Abaqus UEL geometries.

- Loads a UEL plugin (Fortran/C/C++) and calls its `uel_...` entry point for each selected UEL.
- Collects the DOFs for all variables at each element node based on the Abaqus UEL definition.
- Passes nodal coordinates, solution values, increments, and optional external fields (PREDEF) to
  the plugin (provided via AuxVariables, e.g., [AbaqusPredefAux](auxkernels/AbaqusPredefAux.md));
  receives residuals, Jacobian, state variables, and (optionally) energy.
- Executes at `EXEC_PRE_KERNELS` in each nonlinear iteration and inserts residual/Jacobian
  contributions into the system. Contributions are also tagged with the `AbaqusUELTag` vector tag
  for post-processing of concentrated reaction forces.

When using this object, add `AbaqusUELTag` to the problem’s extra tag vectors so that reaction
forces can be accessed by other objects such as
[AbaqusUELStepUserObject](AbaqusUELStepUserObject.md):

```
[Problem]
  extra_tag_vectors = 'AbaqusUELTag'
[]
```

## Interface

The UEL plugin entry function signature is defined in the header:

!listing modules/solid_mechanics/include/userobjects/AbaqusUELMeshUserElement.h start=typedef end=validParams

The `ENERGY` array is only populated when `use_energy = true` is set; state variables are managed
internally and persisted across time steps.

## Usage

- `uel_type`: must match the UEL type name in the Abaqus input (`*User Element, type=...`).
- `element_sets`: one or more Abaqus element sets to operate on; elements are filtered to those of
  the specified `uel_type`.
- `plugin`: path to the compiled UEL plugin; the loader appends a method suffix and `.plugin`.
- `dload_interpolator`: name of the [AbaqusDLoadInterpolator](AbaqusDLoadInterpolator.md) that
  provides per-element `NDLOAD`/`JDLTYP`/`ADLMAG` arrays at the beginning of the timestep. Defaults
  to `dload_uo`.
- `external_fields`: optional list of AuxVariables to pass as PREDEF fields to the UEL. Values are
  typically supplied by [AbaqusPredefAux](auxkernels/AbaqusPredefAux.md).
- `use_energy`: set to true if the UEL writes energy quantities.

### Distributed Loads (DLOAD)

`*Dload` entries in the Abaqus input are parsed per step and pre-interpolated by
[AbaqusDLoadInterpolator](AbaqusDLoadInterpolator.md), which supplies `NDLOAD`, `JDLTYP`, and
`ADLMAG` arrays to this user object. Magnitudes are linearly ramped from the previous step values
to the current step values using the step fraction `d ∈ [0, 1]` from
[AbaqusUELStepUserObject](AbaqusUELStepUserObject.md).

Example configuration (from a test):

!listing modules/solid_mechanics/test/tests/uel_mesh/elastic.i block=UserObjects

!syntax parameters /UserObjects/AbaqusUELMeshUserElement

!syntax inputs /UserObjects/AbaqusUELMeshUserElement

!syntax children /UserObjects/AbaqusUELMeshUserElement
