# MeshCut2DFunctionUserObject

!syntax description /UserObjects/MeshCut2DFunctionUserObject

## Overview

This class is used to define an evolving cutting plane for 2D XFEM simulations based on a mesh that defines an initial crack, and a user-defined function for growth of that crack. It (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 2D elements, and (3) grows the mesh incrementally based on prescribed growth functions defining the direction and growth rate.

## Example Input Syntax

The `[Mesh]` block creating the simulation domain and cutter mesh is shown in [list:mesh]. This mesh block creates two seperate meshes.  The simulation mesh is specified by [!param](/Mesh/MeshGeneratorMesh/final_generator)`=dispBlock`. The cutter mesh is created in the `[cutter_mesh]` and `[move_cutter_mesh]` blocks.  The final mesh cutter block `[move_cutter_mesh]` gives the cutter mesh a name using [!param](/Mesh/FileMeshGenerator/save_with_name) which enables the `MeshCut2DFractureUserObject` shown in [list:cutter] to specify this mesh with [!param](/UserObjects/CrackMeshCut3DUserObject/mesh_generator_name).

!listing test/tests/mesh_cut_2D_fracture/crack_front_stress_function_growth.i id=list:cutter block= UserObjects caption=MeshCut2DFractureUserObject userobject using the cutter mesh created by the `[Mesh]` block in [list:mesh].

!listing test/tests/mesh_cut_2D_fracture/edge_crack_2d_propagation.i id=list:mesh block=Mesh caption=`[Mesh]` block to create the simulation and cutter meshes.

!syntax parameters /UserObjects/MeshCut2DFunctionUserObject

!syntax inputs /UserObjects/MeshCut2DFunctionUserObject

!syntax children /UserObjects/MeshCut2DFunctionUserObject
