# SpiralAnnularMesh

This mesh is derived from MooseMesh. It has an annular shape, and nodes are located on different concentric rings between the inner and outer circles. The elements are triangular.

Here are the required parameters :

- `inner_radius`
- `outer_radius`
- `nodes_per_ring`
- `num_rings`

Given all these parameters, the radial bias will be computed automatically.

It is also possible to specify if you want a second-order Mesh : TRI3 elements will become TRI6 elements. To do that, simply change the `use_tri6` parameter to `true`.

## Example Input File

For example, with the following input file :

```
[Mesh]
  type = SpiralAnnularMesh
  use_tri6 = true
  inner_radius = 1
  nodes_per_ring = 18
  outer_radius = 10
  num_rings = 10
[]
```

The resulting mesh looks like this :

!media large_media/spiral_annular_mesh/SpiralAnnularMesh_example.png
       style=width:50%;

## Further SpiralAnnularMesh Documentation

!syntax parameters /Mesh/SpiralAnnularMesh

!syntax inputs /Mesh/SpiralAnnularMesh

!syntax children /Mesh/SpiralAnnularMesh