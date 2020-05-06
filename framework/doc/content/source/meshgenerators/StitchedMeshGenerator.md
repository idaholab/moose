# StitchedMeshGenerator

!syntax description /MeshGenerators/StitchedMeshGenerator

## Example

Consider the following three meshes.

!media media/mesh/stitched_mesh_left.png style=width:32%;float:left; caption=Fig. 1: Left portion of "stitched" mesh (left.e).

!media media/mesh/stitched_mesh_center.png style=width:32%;float:left;margin-left:2%; caption=Fig. 2: Center portion of "stitched" mesh (center.e).

!media media/mesh/stitched_mesh_right.png style=width:32%;float:right; caption=Fig. 3: Right portion of "stitched" mesh (right.e).

Using the `StitchedMeshGenerator` object from within the [Mesh](/Mesh/index.md) block of the input file, as shown in the input
file snippet below, these three square meshes are joined into a single mesh as shown in Figure 4.

!listing test/tests/meshgenerators/stitched_mesh_generator/stitched_mesh_generator.i block=Mesh

!media media/mesh/stitched_mesh_out.png caption=Fig. 4: Resulting "stitched" mesh from combination of three square meshes.

!syntax parameters /MeshGenerators/StitchedMeshGenerator

!syntax inputs /MeshGenerators/StitchedMeshGenerator

!syntax children /MeshGenerators/StitchedMeshGenerator
