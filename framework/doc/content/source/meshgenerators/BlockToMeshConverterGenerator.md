# BlockToMeshConverterGenerator

!syntax description /Mesh/BlockToMeshConverterGenerator

## Overview

The `BlockToMeshConverterGenerator` moves one (or more) blocks from a mesh to form a new mesh object.
This does not erase/delete the blocks from the original mesh. The new mesh has only one block (unnamed with ID 0)

!alert note
No sidesets are preserved in or added to the new mesh
(neither are any element/node unique id's or ids in general, regardless of what options are set).

## Visual Example

In the input below, six selected blocks are extracted from the input 3D mesh in the first figure.
The final mesh is shown in the second figure.

!listing test/tests/meshgenerators/block_to_mesh_converter_generator/conv_multiblock.i

!media large_media/framework/meshgenerators/block_to_mesh_before.png caption=A 3d object with multiple, multi-colored blocks

!media large_media/framework/meshgenerators/block_to_mesh_after.png caption=A subset of blocks have been made into a new mesh

!syntax parameters /Mesh/BlockToMeshConverterGenerator

!syntax inputs /Mesh/BlockToMeshConverterGenerator

!syntax children /Mesh/BlockToMeshConverterGenerator
