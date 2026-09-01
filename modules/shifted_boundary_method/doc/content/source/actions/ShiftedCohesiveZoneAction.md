# ShiftedCohesiveZoneAction

!syntax description /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction

The `ShiftedCohesiveZoneAction` creates the interface kernels, materials, and optional
auxiliary objects needed by the shifted cohesive zone method. Exactly one of the following
geometry configurations may be used:

- Set `sbm_distance_uo` to an existing `BoundaryShortestDistanceToSurface` object for full
  manual control.
- Set `generate_sbm_distance = true` to create one `SBMSurfaceMeshBuilder` and
  `UnsignedDistanceToSurfaceMesh` per surrogate boundary. Use `surface_meshes` when the
  saved mesh names differ from the boundary names.
- Set `complete_interface_mesh` to one saved mesh containing the complete outward-oriented
  surface of every subdomain. The action creates one
  [`SBMInterfaceManager`](userobjects/SBMInterfaceManager.md) and obtains each interface
  pair from boundary names such as `grain1_grain2` or `block1_block2`.

For boundary names that do not follow the supported naming forms, provide one ordered pair
per boundary with `interface_subdomain_pairs`. The order determines the true interface
normal direction. The source mesh for `complete_interface_mesh` must be retained by its mesh
generator using `save_with_name`.

!syntax parameters /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction
