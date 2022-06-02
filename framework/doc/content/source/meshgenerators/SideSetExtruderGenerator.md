# SideSetExtruderGenerator

!syntax description /Mesh/SideSetExtruderGenerator

## Overview

The SideSetExtruder Generator is for "pulling" mesh in a direction. Given a sideset and a direction(vector), it adds to the mesh. SideSetExtruder differs from [MeshExtruderGenerator](MeshExtruderGenerator.md) in that the extrusion stays in the same dimension as the sideset it is pulling on (MeshExtruderGenerator is for pulling 2 dimensional shapes into 3 dimensions). 

SideSetExtruder will not throw any errors if you extrude a mesh through (collide with) another mesh. It will throw an error if the extrusion vector would create a mesh with a negative determinant (or rather, [MeshExtruderGenerator](MeshExtruderGenerator.md) throws the error). The extrusion vector is applied relative to the sideset given, not from the origin point of the mesh. The extrusion vector must be a 3D vector even if you are only extruding a 2D mesh; in such a case, let the z component be 0 (e.g., `extrusion_vector = '1 0.5 0'`)

The output will have no sidesets, even the sideset you extruded from will be gone. The user is expected to use the other side-set generating generators on the output if sidesets are needed. 

## Visual Example

### Input 2D Mesh

!media large_media/framework/meshgenerators/sideset_extruder_before.png caption=Before: a square, with sidesets on each side.

### Output of SideSetExtruderGenerator

!media large_media/framework/meshgenerators/sideset_extruder_after.png caption=After, the right sideset has been extruded in the < 1, 0.5, 0> direction.

SideSetExtruderGenerator is actually a mere wrapper of 4 other generators: [LowerDBlockFromSidesetGenerator](LowerDBlockFromSidesetGenerator.md), [BlockToMeshConverterGenerator](BlockToMeshConverterGenerator.md), [MeshExtruderGenerator](MeshExtruderGenerator.md), and [StitchedMeshGenerator](StitchedMeshGenerator.md). SideSetExtruderGenerator's output from the example above is exactly equivalent to the output of a recipe like this:

```
[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
  []
  [lowerDblock]
    type = LowerDBlockFromSidesetGenerator
    input = square
    new_block_name = "extrusions0"
    sidesets = "right"
  []
  [separateMesh]
    type = BlockToMeshConverterGenerator
    input = lowerDblock
    target_blocks = extrusions0
  []
  [extrude]
    type = MeshExtruderGenerator
    input = separateMesh
    num_layers = 3
    extrusion_vector = '1 0.5 0'
    bottom_sideset = 'new_bottom'
    top_sideset = 'new_top'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'square extrude'
    stitch_boundaries_pairs = 'right new_bottom'
  []
[]

```

If you are needing to tweak the output of SideSetExtruderGenerator, you may be better off manually running these operations instead. SideSetExtruderGenerator uses the defaults of these sub-generators. 

!syntax parameters /Mesh/SideSetExtruderGenerator

!syntax inputs /Mesh/SideSetExtruderGenerator

!syntax children /Mesh/SideSetExtruderGenerator
