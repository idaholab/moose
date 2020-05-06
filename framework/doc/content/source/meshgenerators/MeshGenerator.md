# MeshGenerator

This is the base class of all the `MeshGenerators` systems.

There are two types of MeshGenerators:

- Those who create a mesh (such as `AnnularMeshGenerator`, `FileMeshGenerator`,...). They are usually similar to the `Mesh` objects.
- Those who modify an existing mesh (such as `MeshExtruderGenerator`, `StitchedMeshGenerator`,...). They are usually similar to `MeshModifier` objects.

The point of these objects is to create complex meshes using only one input file. Indeed, you can use several MeshGenerator blocks in your input file. Those represent the different steps necessary to create your complex mesh.

## Input File Example

For instance, take a look at the following input file:

```
[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
  []

  [./tmg]
    type = TiledMeshGenerator
    input = gmg
    x_width = 1
    y_width = 1
    z_width = 1

    left_boundary = left
    right_boundary = right
    top_boundary = top
    bottom_boundary = bottom
    front_boundary = front
    back_boundary = back

    x_tiles = 2
    y_tiles = 1
    z_tiles = 5
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
```

Structurally, notice that one can have several `MeshGenerator` blocks, but only one `Mesh` block of type `MeshGeneratorMesh`.

Let's analyze the different steps. First, we use a `GeneratedMeshGenerator` to create a 3D mesh labeled `gmg` (here it's simply a regular cube). After that, we use this mesh as an input (via `input = gmg`) to create a larger mesh using a `TiledMeshGenerator`.
