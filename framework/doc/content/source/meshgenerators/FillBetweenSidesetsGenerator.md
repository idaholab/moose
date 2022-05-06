# FillBetweenSidesetsGenerator

!syntax description /Mesh/FillBetweenSidesetsGenerator

## Overview

The `FillBetweenSidesetsGenerator` offers similar functionality to [`FillBetweenPointVectorsGenerator`](/FillBetweenPointVectorsGenerator.md) by leveraging [`FillBetweenPointVectorsTools`](/FillBetweenPointVectorsTools.md). Instead of manually inputting the two boundaries [!param](/Mesh/FillBetweenPointVectorsGenerator/positions_vector_1) and [!param](/Mesh/FillBetweenPointVectorsGenerator/positions_vector_2), The `FillBetweenSidesetsGenerator` directly takes boundary information ([!param](/Mesh/FillBetweenSidesetsGenerator/boundary_1) and [!param](/Mesh/FillBetweenSidesetsGenerator/boundary_2)) of two input meshes, [!param](/Mesh/FillBetweenSidesetsGenerator/input_mesh_1) and [!param](/Mesh/FillBetweenSidesetsGenerator/input_mesh_2). The input meshes can be translated using [!param](/Mesh/FillBetweenSidesetsGenerator/mesh_1_shift) and [!param](/Mesh/FillBetweenSidesetsGenerator/mesh_2_shift). The generated transition layer mesh can be output as a standalone mesh or a stitched mesh with the input meshes, depending on [!param](/Mesh/FillBetweenSidesetsGenerator/keep_inputs).

!media framework/meshgenerators/transition_layer_stitched.png
      style=display: block;margin-left:auto;margin-right:auto;width:80%;
      id=example_tlc
      caption=A typical example of using `FillBetweenSidesetsGenerator` to connect two square meshes together.

If [!param](/Mesh/FillBetweenSidesetsGenerator/keep_inputs) is set as `true`, the original boundaries of the input meshes defined by [!param](/Mesh/FillBetweenSidesetsGenerator/boundary_1) and [!param](/Mesh/FillBetweenSidesetsGenerator/boundary_2) are deleted after stitching the input meshes with the generated transition layer mesh.

All the other meshing options are the same as [`FillBetweenPointVectorsGenerator`](/FillBetweenPointVectorsGenerator.md).

## Example Syntax

!listing test/tests/meshgenerators/fill_between_sidesets_generator/squares.i block=Mesh/fbsg

!syntax parameters /Mesh/FillBetweenSidesetsGenerator

!syntax inputs /Mesh/FillBetweenSidesetsGenerator

!syntax children /Mesh/FillBetweenSidesetsGenerator
