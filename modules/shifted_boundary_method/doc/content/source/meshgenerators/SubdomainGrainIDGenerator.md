# SubdomainGrainIDGenerator

!syntax description /Mesh/SubdomainGrainIDGenerator

The `SubdomainGrainIDGenerator` assigns volume-mesh subdomain IDs from the closed regions in a
saved boundary mesh. For elements cut by one or more grain regions, the selected
`intercepted_subdomain_policy` determines which grain ID is assigned. Elements that are not within any
grain retain their input subdomain ID.

## Performance and Acceleration

The `SubdomainGrainIDGenerator` uses ray casting to determine whether an element lies inside a grain region. To accelerate these containment queries, an internal k-d tree indexes the boundary-element centroids, significantly reducing the number of candidate boundary elements tested for intersection.

### Tuning `leaf_max_size`

The [!param](/Mesh/SubdomainGrainIDGenerator/leaf_max_size) parameter controls the maximum number of boundary-element centroids stored in each leaf node of the k-d tree:
- **Smaller values**: Create a deeper tree with fewer centroids per leaf, which can speed up ray-intersection queries on very fine boundary meshes at the cost of higher tree-build time and memory.
- **Larger values**: Create a shallower tree with more centroids per leaf, reducing tree construction overhead and memory footprint at the cost of more linear intersection checks per leaf.
- **Default (`10`)**: A balanced starting point recommended for most meshes unless performance profiling indicates otherwise.

This parameter is purely a performance tuning parameter; tuning its value will not alter the grain geometry or final subdomain assignments.

For lower-level algorithmic details, see [`AdaptiveRayContainmentCheck`](/AdaptiveRayContainmentCheck.md).

## Example Input File Syntax

The following example demonstrates using `SubdomainGrainIDGenerator` to assign grain subdomain IDs to a generated background mesh based on closed grain boundaries provided by an external boundary mesh:

!listing modules/shifted_boundary_method/test/tests/grain_boundary_case/grain_boundary_sczm_separate_interfaces.i start=[boundary_mesh] end=[break7] include-end=false

!syntax parameters /Mesh/SubdomainGrainIDGenerator

!syntax inputs /Mesh/SubdomainGrainIDGenerator

!syntax children /Mesh/SubdomainGrainIDGenerator
