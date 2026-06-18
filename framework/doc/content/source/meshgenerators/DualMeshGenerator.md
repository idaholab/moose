# DualMeshGenerator

The dual mesh generator creates a dual of a primal mesh. Each node of the dual mesh is positioned at either the circumcenter(s) of a primal element for a Voronoi dual, or the centroid of a primal element for a Barycentric dual. 

For a Voronoi dual, the primal mesh is first circumscribed within a mesh-scaled square, before being Delaunay triangulated using a poly2Tri_Triangulator. Circumcenters are then calculated, and the dual is then trimmed back to match the primal mesh boundary. For Barycentric duals, the dual mesh is calculated directly, then it is extended to match the primal boundary.

!alert note
This mesh generator currently only supports 2D input.

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator
