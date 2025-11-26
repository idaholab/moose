# PolyLineMeshFollowingNodeSetGenerator

!syntax description /Mesh/PolyLineMeshFollowingNodeSetGenerator

Using the `PolyLineMeshFollowingNodeSetGenerator` object from within the
[Mesh](/Mesh/index.md) block of the input file will construct an open
or closed (looped) one-dimensional manifold of Edge elements.

The polyline is generated iteratively, starting from the [!param](/Mesh/PolyLineMeshFollowingNodeSetGenerator/starting_point),
taking the first step following the [!param](/Mesh/PolyLineMeshFollowingNodeSetGenerator/starting_direction):
The following heuristics are implemented for following the nodeset:

- Sphere-based nodeset centroid search
    - the current point is moved by [!param](/Mesh/PolyLineMeshFollowingNodeSetGenerator/dx) towards the current direction
    - the local centroid of all nodes in the target nodeset and closer than [!param](/Mesh/PolyLineMeshFollowingNodeSetGenerator/search_radius) to the current point is found
    - the current direction is updated to be from the current point to the centroid
    - the displacement of the current point is annulled, and replaced by a displacement of magnitude [!param](/Mesh/PolyLineMeshFollowingNodeSetGenerator/dx) towards the centroid
- Ignoring nodes behind, activated with the [!param](/Mesh/PolyLineMeshFollowingNodeSetGenerator/ignore_nodes_behind) parameter
    - only nodes from the nodeset that are also ahead of the current point with the current direction are considered

!alert note
Because of the use of heuristics rather than a global solve, the 1D polyline solution is highly dependent on the parameters chosen.
The user should tune these parameters to fit their needs. Notably, the number of edges can be reduced to prevent the polyline from
backtracking after having reached the end of the nodeset.

!syntax parameters /Mesh/PolyLineMeshFollowingNodeSetGenerator

!syntax inputs /Mesh/PolyLineMeshFollowingNodeSetGenerator
