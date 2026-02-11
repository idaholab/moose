# SBMSurfaceMeshBuilder

`SBMSurfaceMeshBuilder` prepares the geometric data structures required by
[`UnsignedDistanceToSurfaceMesh`](functions/UnsignedDistanceToSurfaceMesh.md) and
other distance-based components. It processes a source mesh—identified by the
`surface_mesh` parameter—to generate a list of boundary element wrappers (such as
`SBMBndEdge2` and `SBMBndTri3`), a map of element IDs, and a KDTree for
efficient nearest-neighbor queries.

To ensure valid spatial searches, the builder verifies that the input mesh is replicated
(serial) and that its dimensionality matches the problem domain (e.g., a 2-D surface
within a 3-D mesh). Users can optionally configure the KDTree leaf size to balance build
time against query speed or enable watertightness checks to validate the boundary
topology. Once constructed, these data structures are exposed for reuse by other objects
in the simulation.

!syntax description /UserObjects/SBMSurfaceMeshBuilder

!syntax parameters /UserObjects/SBMSurfaceMeshBuilder

!syntax inputs /UserObjects/SBMSurfaceMeshBuilder

!syntax children /UserObjects/SBMSurfaceMeshBuilder
