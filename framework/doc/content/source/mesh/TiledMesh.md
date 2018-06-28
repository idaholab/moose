# TiledMesh

!syntax description /Mesh/TiledMesh

## Example

As the name suggests, the `TiledMesh` object may be utilized to repeat a mesh multiple times. The test, [tiled_mesh_test.i](test/tests/mesh/tiled_mesh/tiled_mesh_test.i), will be used to illustrate the use of
the `TiledMesh` object. This test file is shown at the bottom of this section for reference. To execute the test the
moose test application must be compiled:

```bash
cd ~/projects/moose/test
make -j 8
```

Then navigate to the location of the test:

```bash
cd tests/mesh/tiled_mesh
```

!media media/mesh/tiled_mesh_output.png style=float:right;width:25%;margin-left:2%; caption=Fig. 2: Tiled cube created from the cube input.

!media media/mesh/tiled_mesh_input.png style=float:right;width:25%;margin-left:3%; caption=Fig. 1: Cube utilized as input to `TiledMesh` object.

The example utilizes a cube (cube.e) mesh as input as shown in Figure 1, which is a regular cube on the domain from
0 to 10 in the x, y, and z-directions.

As specified in the input file for this test (see below), this mesh is then used to create two tiles in the x, y,
and z directions. To execute the example and create a new mesh, the moose test application is executed with the special "--mesh-only" flag, which indicates that only the mesh operations should be performed. Running this command will create
the resulting mesh file (tiled_mesh_test_in.e), which is intended to be used by a separate input file to run a
complete simulation.

```bash
~/projects/moose/test/moose_test-opt -i tiled_mesh_test.i --mesh-only
```

!listing test/tests/mesh/tiled_mesh/tiled_mesh_test.i


!syntax parameters /Mesh/TiledMesh

!syntax inputs /Mesh/TiledMesh

!syntax children /Mesh/TiledMesh
