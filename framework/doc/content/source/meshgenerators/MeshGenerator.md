# MeshGenerator

This is the base class of all the `MeshGenerators` systems.
There are two types of MeshGenerators :

- Those who create a mesh (such as `AnnularMeshGenerator`, `FileMeshGenerator`, `TriangleMesher`,...). They are usually similar to the `Mesh` objects.
- Those who modify an existing mesh (such as `MeshExtruderGenerator`, `StitchedMeshGenerator`,...). They are usually similar to `MeshModifier` objects.

The point of these objects is to create complex meshes using only one input file. Indeed, you can use several MeshGenerator blocks in your input file. Those represent the different steps necessary to create your complex mesh.

## Input File Example

For instance, let's take a look at the following input file :

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

First of all, one thing to notice is that you have several `MeshGenerator` blocks, but you only have one `Mesh` block, which is of type "MeshGeneratorMesh".

Let's analyse the different steps. First, we use a GeneratedMeshGenerator to create a 3D mesh (here it's simply a regular cube). After that, we use this mesh as an input (that's what `input = gmg` means) to create a larger mesh using a TiledMeshGenerator.