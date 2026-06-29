# DualMeshGenerator

The Dual Mesh Generator creates either a Voronoi or barycentric dual of an input "primal" mesh.

For a barycentric dual, dual nodes are placed at the centroids of each primal element. Around the boundary, primal vertices and midpoints between primal vertices are identified and added to the dual mesh as dual nodes to preserve boundary geometry. For concave boundaries in 2D, dual elements are created as triangles, fanning out from the concave vertex. In 3D, concave boundary elements have an interior node added, from which they are tetrahedralized.

For a Voronoi dual, the primal mesh is first circumscribed within a mesh-scaled square, before being Delaunay triangulated using poly2Tri_Triangulator. Circumcenters of the Delaunay triangles are then calculated and added as dual nodes, and then the mesh is clipped back to match the primal mesh boundary.

!alert note
This mesh generator does not support taking Voronoi duals of 3D meshes.

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator
