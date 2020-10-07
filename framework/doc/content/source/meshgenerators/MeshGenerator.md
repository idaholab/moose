# MeshGenerator

This is the base class of all the `MeshGenerators` systems.

There are two types of MeshGenerators:

- Those who create a mesh (such as `AnnularMeshGenerator`, `FileMeshGenerator`,...). They are usually similar to the `Mesh` objects.
- Those who modify an existing mesh (such as `MeshExtruderGenerator`, `StitchedMeshGenerator`,...). They are usually similar to `MeshModifier` objects.

The point of these objects is to create complex meshes using only one input file. Indeed, you can use several MeshGenerator blocks in your input file. Those represent the different steps necessary to create your complex mesh.

Objects that create a mesh (such as `FileMeshGenerator`,
`GeneratedMeshGenerator`, etc.) should build their libMesh mesh base objects
through `MooseMesh` APIs. This will guarantee that non-local elements will be
handled properly when `prepare_for_use` is called. This is because during
`Action` execution during simulation setup, MOOSE objects such as periodic
boundary conditions or a displaced problem will signal the `MooseMesh`
if they need to delay remote/non-local element removal or if they need certain
remote/non-local elements preserved using the
[`RelationshipManager`](/RelationshipManager.md) system. This information needs
to be communicated to any `MeshGenerators` that are building new meshes in order
for the simulation to run correctly.

There are two `MooseMesh` APIs that should
be used by `MeshGenerators`. `MooseMesh::buildMeshBaseObject` should be called
to construct the mesh that will be returned by the derived class implementation
of `MeshGenerator::generate`. This is a `std::unique_ptr`. If you want to guarantee
that the derived class type of the `std::unique_ptr<MeshBase>` returned by
`_mesh->buildMeshBaseObject()` is a certain type, then
`_mesh->buildMeshBaseObject()` should be preceded by a call to
`_mesh->setParallelType(Moose::ParallelType::REPLICATED)` or
`_mesh->setParallelType(Moose::ParallelType::DISTRIBUTED)`.

The second API, `MooseMesh::buildTypedMesh`, should be used for constructing
auxiliary meshes whose data may be used in the `MeshBase` object that is
returned by `DerivedMeshGenerator::generate`. An example is stitching
meshes. `MooseMesh::buildTypedMesh` takes a single template argument that is the
type of derived `MeshBase` object that you want to build. So if you want a
`ReplicatedMesh`, you would create a `ReplicatedMesh` instance by calling
`_mesh->buildTypedMesh<ReplicatedMesh>(dim)` where `dim` is the desired dimensio
of the mesh. If `dim` is not provided, then `buildTypedMesh` will use the value
of `dim` from the `MooseMesh` `InputParameters` object. Note the difference in
return type between `MooseMesh::buildMeshBaseObject` and
`MooseMesh::buildTypedMesh`. The former will return `std::unique_ptr<MeshBase`
and the latter will return `T` where `T` is the value of the template argument
provided to the `MooseMesh::buildTypedMesh` method.

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
