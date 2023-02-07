# PatternedMeshGenerator

!syntax description /Mesh/PatternedMeshGenerator

## Overview

The `PatternedMeshGenerator` object is similar to [TiledMeshGenerator](/TiledMeshGenerator.md). However, it is restricted to two dimensions and
adds the ability to create a tile pattern from an arbitrary number of input meshes.

!alert note
The MeshGenerators provided in inputs are no longer required to have the same boundary ids for a given boundary name.
In this case, boundary ids for all (cloned) input MeshGenerators are first renamed to a common unused set of ids. Then the resulting stitched patterned mesh will be changed again to boundary ids that are the same as the first entry of the passed inputs parameter.

!alert note
Although best efforts have been made to ensure that the PatternedMeshGenerator fails with a descriptive error message when provided with invalid or ambiguous input, there are some expections that will result in silent failure (the input meshes are stitched together incorrectly and the program continues to run). If this occurs, this issue is likely that one of the left/right/top/bottom_boundary input parameters is set to an incorrect value (i.e., left_boundary = 'right', when the 'right' boundary does not have outward normal (-1, 0, 0)). Check for mistakes like these when troubleshooting errors. Additionally, it should be confirmed that the values of the left/right/top/bottom_boundary parameters are correct for EVERY mesh provided in the 'inputs' parameter (i.e., each mesh in 'inputs' should have the same boundary name, ids may differ, and locations).

For example the input meshes shown in Figures 1 and 2 can be organized into a two dimensional pattern within the input
file, as shown below, to create the pattern shown in Figure 3.

!listing test/tests/meshgenerators/patterned_mesh_generator/patterned_mesh_generator.i block=Mesh

!media media/mesh/quad_mesh.png style=float:left;width:32%; caption=Fig 1: Input put mesh: quad_mesh.e

!media media/mesh/tri_mesh.png style=float:left;width:32%;margin-left:2%; caption=Fig 2: Input put mesh: tri_mesh.e

!media media/mesh/patterned_mesh_in.png style=float:right;width:32%; caption=Fig 3: Resulting mesh created using PatternedMesh.

!syntax parameters /Mesh/PatternedMeshGenerator

!syntax inputs /Mesh/PatternedMeshGenerator

!syntax children /Mesh/PatternedMeshGenerator
