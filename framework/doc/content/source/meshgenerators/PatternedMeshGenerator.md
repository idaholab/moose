# PatternedMeshGenerator

!syntax description /Mesh/PatternedMeshGenerator

## Overview

The `PatternedMeshGenerator` object is similar to [TiledMeshGenerator](/TiledMeshGenerator.md). However, it is restricted to two dimensions and
adds the ability to create a tile pattern from an arbitrary number of input meshes.

For example the input meshes shown in Figures 1 and 2 can be organized into a two dimensional pattern within the input
file, as shown below, to create the pattern shown in Figure 3.

!listing test/tests/meshgenerators/patterned_mesh_generator/patterned_mesh_generator.i block=Mesh

!media media/mesh/quad_mesh.png style=float:left;width:32%; caption=Fig 1: Input put mesh: quad_mesh.e

!media media/mesh/tri_mesh.png style=float:left;width:32%;margin-left:2%; caption=Fig 2: Input put mesh: tri_mesh.e

!media media/mesh/patterned_mesh_in.png style=float:right;width:32%; caption=Fig 3: Resulting mesh created using PatternedMesh.

The parameters [!param](/Mesh/PatternedMeshGenerator/top_boundary), [!param](/Mesh/PatternedMeshGenerator/bottom_boundary), [!param](/Mesh/PatternedMeshGenerator/left_boundary), and [!param](/Mesh/PatternedMeshGenerator/right_boundary) are provided by the user to specify the top, bottom, left, and right sideset names respectively of the input meshes, which are used for stitching the input meshes into the output patterned mesh. The names of the top, bottom, left, and right sidesets of the patterned mesh can be specified by the optional parameters [!param](/Mesh/PatternedMeshGenerator/new_top_boundary), [!param](/Mesh/PatternedMeshGenerator/new_bottom_boundary), [!param](/Mesh/PatternedMeshGenerator/new_left_boundary), and [!param](/Mesh/PatternedMeshGenerator/new_right_boundary), respecitvely. If these parameters are not specified, then the boundaries specified by [!param](/Mesh/PatternedMeshGenerator/top_boundary), [!param](/Mesh/PatternedMeshGenerator/bottom_boundary), [!param](/Mesh/PatternedMeshGenerator/left_boundary), and [!param](/Mesh/PatternedMeshGenerator/right_boundary) will be deleted and used to set the outer boundaries of the output patterned mesh instead.

!syntax parameters /Mesh/PatternedMeshGenerator

!syntax inputs /Mesh/PatternedMeshGenerator

!syntax children /Mesh/PatternedMeshGenerator
