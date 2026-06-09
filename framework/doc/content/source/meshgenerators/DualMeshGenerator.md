# DualMeshGenerator

The dual mesh generator creates a Voronoi dual of a primal mesh. The nodes of the dualmeshare positioned at the circumcenters or each primal element, and form polygonal elements.

On the boundaries, additional nodes are placed at the mipoints on the primal mesh boundary sides, and the boundary vertices of the primal mesh. Centroids are used in lieu of circumcenters in cases where circumcenters lie outside of polygon boundaries.

!alert node
This mesh generator currently only supports 2D input.

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator