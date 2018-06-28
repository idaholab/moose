# PatternedMesh

!syntax description /Mesh/PatternedMesh

The `PatternedMesh` object is similar to [TiledMesh](/TiledMesh.md) but restricted to two dimensions but
adds the ability to create a tile pattern from an arbitrary number of input meshes.

For example the input meshes shown in Figures 1 and 2 can be organized into a two dimensional pattern within the input
file, as shown below, to create the pattern shown in Figure 3.

!listing test/tests/mesh/patterned_mesh/patterned_mesh.i

!media media/mesh/quad_mesh.png style=float:left;width:32%; caption=Fig 1: Input put mesh: quad_mesh.e

!media media/mesh/tri_mesh.png style=float:left;width:32%;margin-left:2%; caption=Fig 2: Input put mesh: tri_mesh.e

!media media/mesh/patterned_mesh_in.png style=float:right;width:32%; caption=Fig 3: Resulting mesh created using PatternedMesh.


!syntax parameters /Mesh/PatternedMesh

!syntax inputs /Mesh/PatternedMesh

!syntax children /Mesh/PatternedMesh
