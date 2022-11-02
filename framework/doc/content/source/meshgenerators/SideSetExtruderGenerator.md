# SideSetExtruderGenerator

!syntax description /Mesh/SideSetExtruderGenerator

The `SideSetExtruderGenerator` differs from the [MeshExtruderGenerator](MeshExtruderGenerator.md) in that the extruded
block has the same dimensionality as the input mesh.

!alert note
The `SideSetExtruderGenerator` will not throw any errors if you extrude a mesh through (overlap with) another mesh or
another part of the mesh. It will throw an error if the extrusion vector would create a mesh with a negative determinant
(the nested [MeshExtruderGenerator](MeshExtruderGenerator.md) throws the error).

!alert warning
The output will have no sidesets, even the sideset which served for extrusion will be removed.
The user is expected to use the other sideset generating generators on the output if sidesets are needed. 

## Visual Example

The following 2D mesh has the `right` sideset extruded using a `SideSetExtruderGenerator` with the (1, 0.5, 0) vector.

!media large_media/framework/meshgenerators/sideset_extruder_before.png caption=Before: a square, with sidesets on each side.

!media large_media/framework/meshgenerators/sideset_extruder_after.png caption=After, the right sideset has been extruded.

## Implementation details

SideSetExtruderGenerator is actually a mere wrapper of 4 other generators:

- a [LowerDBlockFromSidesetGenerator](LowerDBlockFromSidesetGenerator.md) to generate a block from the sideset
- a [BlockToMeshConverterGenerator](BlockToMeshConverterGenerator.md) to generate a block with the mesh
- a [MeshExtruderGenerator](MeshExtruderGenerator.md) to extrude the new mesh
- a [StitchedMeshGenerator](StitchedMeshGenerator.md) to stitch the original mesh and the extruded mesh.


As such, the `SideSetExtruderGenerator` is exactly equivalent to the output of a recipe similar to the one below.
If you are needing to tweak the output of `SideSetExtruderGenerator`, it may be preferrable to use these generators
instead. `SideSetExtruderGenerator` uses the default parameters of these sub-generators. 

!listing test/tests/meshgenerators/sideset_extruder_generator/extrude_square.i

The input above should be equivalent to the input shown below.

!listing test/tests/meshgenerators/sideset_extruder_generator/external_generators.i

!syntax parameters /Mesh/SideSetExtruderGenerator

!syntax inputs /Mesh/SideSetExtruderGenerator

!syntax children /Mesh/SideSetExtruderGenerator
