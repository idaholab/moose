# SurfaceMeshBySubdomainBuilder

`SurfaceMeshBySubdomainBuilder` prepares the geometric data structures used by
[`PointInSubdomainCheckUO`](userobjects/PointInSubdomainCheckUO.md) for per-subdomain
in-out testing. It processes a source surface mesh, identified by the `surface_mesh`
parameter, and groups its boundary elements into one `SurfaceElementSet` per subdomain ID.
The per-subdomain sets are exposed for reuse by other objects in the simulation.

As a subclass of [`BoundaryMeshBuilder`](userobjects/BoundaryMeshBuilder.md), it retrieves
and validates the saved surface mesh: the mesh must be replicated (serial), and its
dimension must be one less than the embedding mesh (e.g., a 2-D surface within a 3-D mesh).
The KDTree used for nearest-neighbor queries is not built here; it is constructed by the
consuming in-out tester ([`PointInSubdomainCheckUO`](userobjects/PointInSubdomainCheckUO.md))
from these sets.

## Usage

Set `surface_mesh` to the name under which the surface mesh was stored using the
`MeshGenerator` `save_with_name` parameter. The replicated-mesh check is always performed.
Because the boundary loops must be closed for a reliable in-out test, consider enabling
`check_watertightness` when preparing a new mesh.

!syntax description /UserObjects/SurfaceMeshBySubdomainBuilder

!syntax parameters /UserObjects/SurfaceMeshBySubdomainBuilder

!syntax inputs /UserObjects/SurfaceMeshBySubdomainBuilder

!syntax children /UserObjects/SurfaceMeshBySubdomainBuilder
