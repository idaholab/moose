# SetupPeriodicRayBCAction

!syntax description /RayBCs/SetupPeriodicRayBCAction

It acts on all [RayBCs/index.md], but only operates on those that are a [PeriodicRayBC.md].

A separate action is required for a [PeriodicRayBC.md] as ghosting needs to be setup before
the [PeriodicRayBC.md] is constructed. This action sets up that ghosting and the
`libMesh::PeriodicBoundaries` object needed by the boundary condition.
