# ShiftedCohesiveZoneAction

!syntax description /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction

The `ShiftedCohesiveZoneAction` creates the interface kernels, displacement-jump and global-traction
materials, and optional geometry objects needed by the shifted cohesive zone method. Only
small-strain kinematics are currently supported.

Setting up an SCZM problem involves two independent choices:

1. Select how the action obtains the distance and true normal for each interface.
2. Define a constitutive traction material on the same surrogate boundaries.

## Select the interface geometry

The `boundary` parameter always lists the surrogate boundaries on which SCZM is applied. Exactly
one of the following three approaches supplies their corresponding true-interface geometry.

### Use an existing distance user object

Set `sbm_distance_uo` to an existing `BoundaryShortestDistanceToSurface` for full control over the
distance calculation. The following example defines the object from a level-set function:

!listing modules/shifted_boundary_method/test/tests/shifted_cohesive_zone/angled_sczm_nonad_salehani.i start=[UserObjects] end=[Functions] include-end=false

The action can receive the object name directly or inherit it from `[GlobalParams]`, as this example
does:

!listing modules/shifted_boundary_method/test/tests/shifted_cohesive_zone/angled_sczm_nonad_salehani.i start=[GlobalParams] end=[Problem] include-end=false

This approach is the most flexible, but the user is responsible for constructing the distance user
object and any functions or surface data it needs.

### Use one surface mesh per interface

Set `generate_sbm_distance = true` when a separate saved surface mesh is available for every
interface. The action creates the surface-mesh builders, distance functions, and aggregate boundary
distance user object. If each saved mesh has the same name as its corresponding surrogate boundary,
only the following Physics configuration is needed:

!listing modules/shifted_boundary_method/test/tests/grain_boundary_case/grain_boundary_sczm_separate_interfaces.i start=[Physics/SolidMechanics/ShiftedCohesiveZone] end=[Materials] include-end=false

Set `surface_meshes` only when the saved mesh names differ from the boundary names. Entries in
`surface_meshes` and `boundary` correspond by position. For example:

```
boundary = 'grain1_grain2 grain1_grain4'
surface_meshes = 'surface_12 surface_14'
```

Here, `surface_12` supplies the geometry for `grain1_grain2`, and `surface_14` supplies the geometry
for `grain1_grain4`.

### Use one complete interface mesh

Set `complete_interface_mesh` to one saved mesh containing the complete outward-oriented boundary
of every subdomain. The action creates one
[`SBMInterfaceManager`](userobjects/SBMInterfaceManager.md) and one boundary distance user object:

!listing modules/shifted_boundary_method/test/tests/grain_boundary_case/grain_boundary_sczm_coarse_lambda1.i start=[Physics/SolidMechanics/ShiftedCohesiveZone] end=[Materials] include-end=false

The mesh generator that supplies `complete_interface_mesh` must retain the mesh with
`save_with_name`. Boundary names of the form `grainX_grainY` or `blockX_blockY` identify the two
interface subdomains automatically. For other boundary names, set `interface_subdomain_pairs` with
one pair per boundary in the same order. The pair selects an interface; its order does not control
the final true-normal orientation.

The `no_shifted = true` option disables shifted evaluation and does not provide another geometry
source.

## Define the traction law

The action does not create the constitutive traction law. Define an interface material such as
`BiLinearMixedModeTraction` or `SalehaniIrani3DCTraction` and restrict it to the same surrogate
boundaries as the SCZM Physics. The action-created displacement-jump and global-traction materials
then use the properties produced by this constitutive material.

For multiple interfaces, define the boundary list once with HIT substitution and use it in both
places. The complete-interface example uses this pattern:

!listing modules/shifted_boundary_method/test/tests/grain_boundary_case/grain_boundary_sczm_coarse_lambda1.i start=interfaces end=ny include-end=false

!listing modules/shifted_boundary_method/test/tests/grain_boundary_case/grain_boundary_sczm_coarse_lambda1.i start=[Physics/SolidMechanics/ShiftedCohesiveZone] end=[Executioner] include-end=false

Keeping the traction material explicit allows different constitutive laws to be assigned to
different subsets of the SCZM boundaries. The action sets `boundary`, `displacements`, and
`base_name` only on the kernels and materials that it creates; it does not modify user-defined
materials in `[Materials]`.

## Non-AD tangent definition

The non-AD interface kernel uses `stress` and `tangent` material properties for its directional
correction and Jacobian. When `tangent_definition = auto`, the standard `Jacobian_mult` property is
interpreted as $\partial\boldsymbol{\sigma}/\partial\boldsymbol{\varepsilon}$ and
`pk1_jacobian` is interpreted as $\partial\mathbf{P}/\partial\mathbf{F}$. A custom `tangent`
property name requires an explicit `tangent_definition`:

- Use `stress_wrt_strain` only for a derivative of stress with respect to symmetric strain ($\partial\boldsymbol{\sigma}/\partial\boldsymbol{\varepsilon}$).
- Use `pk1_wrt_deformation_gradient` only for a derivative of first Piola-Kirchhoff stress with respect to the full deformation gradient ($\partial\mathbf{P}/\partial\mathbf{F}$).

The AD interface kernel differentiates its residual automatically and does not use `tangent` or
`tangent_definition`; setting either parameter with AD enabled is an error.

!syntax parameters /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction
