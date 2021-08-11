# Peridynamics Mesh Generator

## Description

The `MeshGeneratorPD` MeshGenerator selectively converts finite element meshes and boundary sets to peridynamics meshes and corresponding boundary sets. For cases when more than one element block exist in the finite element mesh, user can specify which element block to convert to peridynamics elements and whether the converted finite elements will be retained with the newly created peridynamics elements or not.

For boundary sideset conversion, phantom elements are constructed based on peridynamics material points that were converted from finite elements adjacent to the specified FE boundary sidesets. For 2D FE meshes of either triangular or quadrilateral finite elements, three-node triangular phantom elements are constructed. For 3D FE meshes, the sideset construction currently only accepts hexahedral elements.

!syntax parameters /Mesh/MeshGeneratorPD

!syntax inputs /Mesh/MeshGeneratorPD

!syntax children /Mesh/MeshGeneratorPD
