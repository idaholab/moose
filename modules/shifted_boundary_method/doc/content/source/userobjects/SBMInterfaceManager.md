# SBMInterfaceManager

`SBMInterfaceManager` detects shared interfaces in one saved surface mesh containing the
complete boundary of each subdomain. It groups surface elements by subdomain ID, matches
coincident faces with the same vertices between each pair, and builds a KDTree for distance and
normal queries on each detected interface. The interface meshes must therefore be conformal; partial
face overlaps are not treated as complete interface elements.

The source mesh must be replicated, have dimension one less than the solving mesh, and use
subdomain IDs that identify the enclosed regions. Each per-subdomain surface must use
outward-facing element normals. A query for `(first, second)` returns the normal from the
first subdomain toward the second; reversing the pair reverses that normal.

Set `complete_interface_mesh` to a mesh retained by a `MeshGenerator` with `save_with_name`. The
`tolerance` parameter is relative to the largest dimension of the complete surface mesh and
controls coincident-face matching. `normal_tolerance` controls the parallel-normal check.

The manager can be queried directly by
[`UnsignedDistanceToSurfaceMesh`](functions/UnsignedDistanceToSurfaceMesh.md), or used by
the shifted cohesive zone action through its `complete_interface_mesh` parameter.

!syntax description /UserObjects/SBMInterfaceManager

!syntax parameters /UserObjects/SBMInterfaceManager

!syntax inputs /UserObjects/SBMInterfaceManager

!syntax children /UserObjects/SBMInterfaceManager
