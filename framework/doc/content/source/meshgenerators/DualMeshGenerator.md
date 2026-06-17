# DualMeshGenerator

The dual mesh generator creates a Voronoi dual of a primal mesh. The nodes of the dual mesh are positioned at the circumcenters or each primal element, and form polygonal elements.

The primal mesh is first circumscribed within a large square, before being Delaunay triangulated using a poly2Tri_Triangulator. Circumcenters are then calculated, and the dual is then trimmed back to match the primal mesh boundary. 

!alert node
This mesh generator currently only supports 2D input.

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator