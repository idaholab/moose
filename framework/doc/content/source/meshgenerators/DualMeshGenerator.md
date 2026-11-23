# DualMeshGenerator

!syntax description /Mesh/DualMeshGenerator

## Overview

The `DualMeshGenerator` object creates either a Voronoi or barycentric dual of an input "primal" mesh. In the generated dual mesh, input nodes become dual elements and input elements provide the locations for dual nodes.

For a barycentric dual, dual nodes are placed at the centroids of the primal elements. Primal boundary vertices and boundary edge midpoints are added where needed so the dual mesh preserves the primal boundary geometry. Around concave boundaries, dual elements may be created by fanning from the concave vertex.

For a Voronoi dual, the primal mesh is first enclosed in a mesh-scaled square and then Delaunay triangulated. The circumcenters of the Delaunay triangles are used as dual nodes, and the dual mesh is clipped back to the primal mesh boundary.

!alert note
This mesh generator currently only supports 2D input.

The [!param](/Mesh/DualMeshGenerator/preserve_subdomain_interfaces) parameter treats interfaces between primal subdomains like preserved external boundaries. This creates separate dual cells on each side of an interface instead of merging the cells across subdomain boundaries. This option is only supported for barycentric duals.

The [!param](/Mesh/DualMeshGenerator/preserve_primal_subdomains) parameter copies selected primal subdomains to the output mesh while dualizing the rest of the mesh. Preserved primal subdomains are only supported for barycentric duals, and the associated nodesets and sidesets are copied to the output mesh.

Sidesets and nodesets on the external boundary of the primal mesh are transferred to the dual mesh.
Each dual element side takes the boundary ids of the primal side containing it, and a nodeset that
covers a whole primal side is extended to the dual nodes introduced along that side. Sidesets on a
subdomain interface are transferred in the same way when
[!param](/Mesh/DualMeshGenerator/preserve_subdomain_interfaces) is set, since the dual elements then
stop at the interface; on an interface the ids are placed on one side of the pair, as in the primal
mesh.

!alert warning
Sidesets and nodesets interior to a subdomain are not preserved, because the dual elements do not
in general have sides along them.

## Example Syntax

!listing test/tests/meshgenerators/dual_mesh_generator/dual_mesh_generator_barycentric.i block=Mesh

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator
