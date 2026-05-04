# CrackMeshCut3DUserObject

!syntax description /UserObjects/CrackMeshCut3DUserObject

## Overview

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 3D elements, and (3) grows the mesh incrementally based on prescribed growth functions. The code is interfaced with domain integral methods to allow nonplanar crack growth based on empirical propagation direction and speed laws.

For crack-growth problems, [!param](/UserObjects/CrackMeshCut3DUserObject/size_control) sets the target spacing used when advancing and refining the cutter front, while[!param](/UserObjects/CrackMeshCut3DUserObject/min_elem_area) prevents the growth algorithm from adding very small or nearly degenerate triangles to the cutter mesh. The number of growth advances performed in one update is controlled by [!param](/UserObjects/CrackMeshCut3DUserObject/n_step_growth).

## Example Input Syntax

This example shows the `Mesh` block in [list:mesh] needed for creating the cutter mesh along with the `CrackMeshCut3DUserObject` block in [list:cutobject].  The mesh block in [list:mesh] defines two separate meshes.  The cutter mesh is created in the `read_in_cutter_mesh` block and must have [!param](/Mesh/FileMeshGenerator/save_with_name) set in order to specify this mesh in `CrackMeshCut3DUserObject` shown in [list:cutobject] using [!param](/UserObjects/CrackMeshCut3DUserObject/mesh_generator_name).  The mesh used by the FEM simulation is specifed in the `FEM_mesh` block in this example and [!param](/Mesh/MeshGeneratorMesh/final_generator)`=FEM_mesh` must be set because only the `FEM_mesh` will be used for the finite-element solution and the mesh created by `read_in_cutter_mesh` will be ignored by the solution.

!listing test/tests/solid_mechanics_basic/edge_crack_3d_domain.i id=list:mesh block=Mesh caption=Setting up the mesh block contain simulation and cutter meshes.

!listing test/tests/solid_mechanics_basic/edge_crack_3d_mhs.i id=list:cutobject block=UserObjects caption=CrackMeshCut3DUserObject that uses the cutter mesh created in [list:cutobject].

!syntax parameters /UserObjects/CrackMeshCut3DUserObject

!syntax inputs /UserObjects/CrackMeshCut3DUserObject

!syntax children /UserObjects/CrackMeshCut3DUserObject
