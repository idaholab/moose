# SpiralAnnularMeshGenerator

!syntax description /Mesh/SpiralAnnularMeshGenerator

## Overview

The generated mesh has an annular shape, and nodes are located on different concentric rings between the inner and outer circles. The elements are triangular.

Here are the required parameters:

- `inner_radius`
- `outer_radius`
- `nodes_per_ring`
- `num_rings`

Given all these parameters, the radial bias will be computed automatically.

It is also possible to specify if you want a second-order Mesh : TRI3 elements will become TRI6 elements. To do that, simply change the `use_tri6` parameter to `true`.

## Example Input File

For example, with the following input file block:

!listing test/tests/meshgenerators/spiral_annular_mesh_generator/spiral_annular_mesh_generator.i block=Mesh

The resulting mesh looks like this:

!media large_media/spiral_annular_mesh/SpiralAnnularMesh_example.png
       style=width:50%;

!syntax parameters /Mesh/SpiralAnnularMeshGenerator

!syntax inputs /Mesh/SpiralAnnularMeshGenerator

!syntax children /Mesh/SpiralAnnularMeshGenerator
