# SphereMeshGenerator

!syntax description /Mesh/SphereMeshGenerator

## Overview

The `SphereMeshGenerator` creates a mesh of a sphere (a ball, not its boundary). The mesh refinement and smoothing parameters define the mesh density. [fig:sphere_mesh] depicts several sphere meshes (sliced in half to illustrate the interior elements) for different selections of refinement and smoothing parameters.

!media large_media/framework/meshgenerators/sphere_mesh.png id=fig:sphere_mesh
       caption=Sphere meshes created with different choices for refinement level and smoothing operations

The dimension of the generated mesh is determined by the selected
`elem_type`.  3D element types will cause a ball volume to be meshed,
2D element types will cause a circular area (disk) to be meshed, and 1D
element types will cause a line segment to be meshed.

!syntax parameters /Mesh/SphereMeshGenerator

!syntax inputs /Mesh/SphereMeshGenerator

!syntax children /Mesh/SphereMeshGenerator
