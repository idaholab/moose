# DualMeshGenerator

!syntax description /Mesh/DualMeshGenerator

## Overview

The `DualMeshGenerator` object creates either a Voronoi or barycentric dual of an input "primal" mesh. In the generated dual mesh, input nodes become dual elements and input elements provide the locations for dual nodes.

For a barycentric dual, dual nodes are placed at the centroids of the primal elements. Primal boundary vertices and boundary edge midpoints are added where needed so the dual mesh preserves the primal boundary geometry. Around concave boundaries, dual elements may be created by fanning from the concave vertex.

For a Voronoi dual, the primal mesh is first enclosed in a mesh-scaled square and then Delaunay triangulated. The circumcenters of the Delaunay triangles are used as dual nodes, and the dual mesh is clipped back to the primal mesh boundary.

!alert note
This mesh generator currently only supports 2D input.

The [!param](/Mesh/DualMeshGenerator/preserve_subdomain_interfaces) parameter treats interfaces between primal subdomains like preserved external boundaries. This creates separate dual cells on each side of an interface instead of merging the cells across subdomain boundaries. This option is not supported for Voronoi duals with multiple subdomains.

The [!param](/Mesh/DualMeshGenerator/preserve_primal_subdomains) parameter copies selected primal subdomains to the output mesh while dualizing the rest of the mesh. Preserved primal subdomains are only supported for barycentric duals, and the associated nodesets and sidesets are copied to the output mesh.

## Example Syntax

!listing test/tests/meshgenerators/dual_mesh_generator/dual_mesh_generator_barycentric.i block=Mesh

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator
