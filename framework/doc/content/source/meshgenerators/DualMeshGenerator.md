# DualMeshGenerator

!syntax description /Mesh/DualMeshGenerator

## Overview

The `DualMeshGenerator` object creates either a Voronoi or barycentric dual of an input "primal"
mesh. In the generated dual mesh, input nodes become dual elements and input elements provide the
locations for dual nodes.

Input sideset and nodeset names are retained. Sideset IDs are transferred to generated dual sides
that lie on the corresponding primal sides, and nodeset IDs are retained for primal nodes that are
present in the output mesh.

For a barycentric dual, dual nodes are placed at the centroids of the primal elements. Primal
boundary vertices and boundary edge midpoints are added where needed so the dual mesh preserves the
primal boundary geometry.

For a Voronoi dual, the primal mesh is first enclosed in a mesh-scaled square and then Delaunay
triangulated. The circumcenters of the Delaunay triangles are used as dual nodes, and the dual mesh
is clipped back to the primal mesh boundary.

!alert note
This mesh generator does not support taking Voronoi duals of 3D meshes.

In 3D, barycentric dual polyhedra may be non-convex near boundaries, reentrant corners, or preserved
interfaces. The [!param](/Mesh/DualMeshGenerator/concave_treatment) parameter controls the ordered
treatments used for these cells. The available treatments can split the polyhedron, cut it into
convex child polyhedra, or tetrahedralize it with NetGen.

The [!param](/Mesh/DualMeshGenerator/preserve_subdomain_interfaces) parameter treats interfaces
between primal subdomains like preserved external boundaries. The
[!param](/Mesh/DualMeshGenerator/preserve_primal_subdomains) parameter copies selected primal
subdomains to the output mesh while dualizing the rest of the mesh. Preserved primal subdomains are
supported for barycentric duals in 2D and 3D, and the associated nodesets and sidesets are copied to
the output mesh.

## Example Syntax

!listing test/tests/meshgenerators/dual_mesh_generator/dual_mesh_generator_2d_barycentric.i block=Mesh

!syntax parameters /Mesh/DualMeshGenerator

!syntax inputs /Mesh/DualMeshGenerator

!syntax children /Mesh/DualMeshGenerator
