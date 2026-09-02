# ShiftedCohesiveZoneAction

!syntax description /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction

The `ShiftedCohesiveZoneAction` creates the interface kernels, materials, and optional
auxiliary objects needed by the shifted cohesive zone method. Only small-strain kinematics are
currently supported.

Exactly one of the following geometry configurations may be used:

- Set `sbm_distance_uo` to an existing `BoundaryShortestDistanceToSurface` object for full
  manual control.
- Set `generate_sbm_distance = true` to create one `SBMSurfaceMeshBuilder` and
  `UnsignedDistanceToSurfaceMesh` per surrogate boundary. Use `surface_meshes` when the
  saved mesh names differ from the boundary names.
- Set `complete_interface_mesh` to one saved mesh containing the complete outward-oriented
  surface of every subdomain. The action creates one
  [`SBMInterfaceManager`](userobjects/SBMInterfaceManager.md) and obtains each interface
  pair from boundary names such as `grain1_grain2` or `block1_block2`.

For boundary names that do not follow the supported naming forms, provide one subdomain pair
per boundary with `interface_subdomain_pairs`. The pair identifies the interface; the true normal
is oriented automatically so that its integrated area correction factor is positive. The source
mesh for `complete_interface_mesh` must be retained by its mesh generator using `save_with_name`.

## Non-AD tangent definition

The non-AD interface kernel uses `stress` and `tangent` material properties for its directional
correction and Jacobian. When `tangent_definition = auto`, the standard `Jacobian_mult` property is
interpreted as $\partial\boldsymbol{\sigma}/\partial\boldsymbol{\varepsilon}$ and
`pk1_jacobian` is interpreted as $\partial\mathbf{P}/\partial\mathbf{F}$. A custom `tangent`
property name requires an explicit `tangent_definition`:

- Use `stress_wrt_strain` only for a derivative of stress with respect to symmetric strain.
- Use `pk1_wrt_deformation_gradient` only for a derivative of first Piola-Kirchhoff stress with
  respect to the full deformation gradient.

The AD interface kernel differentiates its residual automatically and does not use `tangent` or
`tangent_definition`; setting either parameter with AD enabled is an error.

!syntax parameters /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction
