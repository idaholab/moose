# SurfaceMeshBySubdomainBuilder

`SurfaceMeshBySubdomainBuilder` prepares the geometric data structures used by
[`PointInSubdomainCheckUO`](userobjects/PointInSubdomainCheckUO.md) for per-subdomain
in-out testing. It processes a source surface mesh, identified by the `surface_mesh`
parameter, and groups its boundary elements, element IDs, and centroids by subdomain ID,
building a KDTree per subdomain for efficient nearest-neighbor queries.

To ensure valid spatial searches, the builder verifies that the input mesh is replicated
(serial) and that its dimensionality is one less than the embedding mesh (e.g., a 2-D
surface within a 3-D mesh). Users can optionally configure the KDTree leaf size to balance
build time against query speed, or enable a watertightness check to validate the boundary
topology. Once constructed, the subdomain-grouped data structures are exposed for reuse by
other objects in the simulation.

## Usage

Set `surface_mesh` to the name under which the surface mesh was stored using the
`MeshGenerator` `save_mesh_as` parameter. Because the boundary loops must be closed for a
reliable in-out test, keep `check_replicated` enabled and consider enabling
`check_watertightness` when preparing a new mesh.

!syntax description /UserObjects/SurfaceMeshBySubdomainBuilder

!syntax parameters /UserObjects/SurfaceMeshBySubdomainBuilder

!syntax inputs /UserObjects/SurfaceMeshBySubdomainBuilder

!syntax children /UserObjects/SurfaceMeshBySubdomainBuilder
