# MeshGenerator

This is the base class for the `MeshGenerator` system.

There are two types of mesh generators:

- Those who create a mesh (examples: [GeneratedMeshGenerator.md], [AnnularMeshGenerator.md], [FileMeshGenerator.md]).
- Those who modify an existing mesh (examples: [MeshExtruderGenerator.md], [StitchedMeshGenerator.md], [RenameBlockGenerator.md], [RenameBoundaryGenerator.md]).

The purpose of these objects is to create complex meshes using only one input file. Indeed, you can use several MeshGenerator blocks in your input file. Those represent the different steps necessary to create your complex mesh. Note that an option exists for printing mesh information to screen (see [#showing-mesh-information]) for tracking down problems when generating complex meshes with multiple mesh generators.

## Input File Example

```
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
  []

  [tmg]
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

[Outputs]
  exodus = true
[]
```

Let's analyze the different steps. First, we use a [GeneratedMeshGenerator.md] to create a 3D mesh labeled `gmg` (here it's simply a regular cube). After that, we use this mesh as an input (via `input = gmg`) to create a larger mesh using a [TiledMeshGenerator.md].

## Developing a MeshGenerator

For mesh generators that create a mesh, care must be taken as to the parallel type of the mesh. You will override the `generate()` method, obtain a base mesh object, and then return your generated mesh. For obtaining the base mesh object, there are three options:

- If the mesh to be generated is agnostic to the parallel type (that is, it can be either replicated or distributed), use `MeshGenerator::buildMeshBaseObject()` to generate the base mesh. See [GeneratedMeshGenerator.md] for an example.
- If the mesh to be generated is only replicated, use `MeshGenerator::buildReplicatedMesh()` to generate the base mesh.
- If the mesh to be generated is only distributed, use `MeshGenerator::buildDistributedMesh()` to generate the base mesh. See [DistributedRectilinearMeshGenerator.md] for an example.

You should +only+ use these APIs to create the base mesh in generators that create meshes. This is very important, because the internal preparation of the meshes for use is dependent on the parallel type of the mesh.

For mesh generators that modify an existing mesh, you should have as an input parameter a `MeshGeneratorName` (or multiple, as a `std::vector<MeshGeneratorName>` if applicable) to obtain the mesh(es) to modify. You can then obtain said meshes via `MeshGenerator::getMesh()` and `MeshGenerator::getMeshByName()`. For examples, see [RenameBoundaryGenerator.md] and [StitchedMeshGenerator.md]. You then act on said meshes by overriding the `generate()` method, and returning the resulting mesh.

### Using Sub MeshGenerators

Your mesh generator can instantiate subgenerators itself, to create existing meshes for the primary generator to combine and/or modify.  The easiest way to do this is to call `MeshGenerator::addMeshSubgenerator()`, which takes as arguments the generator class name and object name to instantiate, followed by an arbitrary number of name/value pairs of subgenerator parameters to set. Sub generators can be chained together by coupling them via input parameters, that is, setting input names as the names of other sub generators. To obtain a mesh from a sub generator, use the same `MeshGenerator::getMeshByName()` API as described above but with the name of the created sub generator.

If you wish to use an input mesh from the primary generator as an input to a sub generator, you must first call `MeshGenerator::declareMeshForSub()` or `MeshGenerator::declareMeshForSubByName()` to declare said input as a dependency for the sub generator instead of the generator that creates the sub generator.

### Declaring a Null or Dummy Input

By default, the MeshGenerator system assumes that all parameters of type `MeshGeneratorName` or `std::vector<MeshGeneratorName>` are to be used as mesh inputs. If one of said names is not found, an error will be reported stating that the input mesh cannot be found. There exist cases where you want to allow the user to provide a "null" or dummy value for an input. An example is letting the user define a hole in a mesh pattern.

You may do this by using the `MeshGenerator::declareNullMeshName()` API. This API takes a name that is to be considered unused as an input mesh. Any input with this name will not be searched for as an existing MeshGenerator mesh and will return an uninitialized mesh when using the input mesh getters (`MeshGenerator::getMesh()`, etc).

## Showing Mesh Information

The parameter [!param](/Mesh/GeneratedMeshGenerator/show_info) prints to screen detailed information about a mesh immediately after it is generated. It can be used to quickly verify properties about the mesh such as: sidesets, subdomains, and volumes.

Take the simple example:

```
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    show_info = true
  []
```

The above will result in on-screen output similar to the following (ran with two processes):

```
GeneratedMeshGenerator 'gmg':  Mesh Information:
GeneratedMeshGenerator 'gmg':   elem_dimensions()={2}
GeneratedMeshGenerator 'gmg':   spatial_dimension()=2
GeneratedMeshGenerator 'gmg':   n_nodes()=25
GeneratedMeshGenerator 'gmg':     n_local_nodes()=15
GeneratedMeshGenerator 'gmg':   n_elem()=16
GeneratedMeshGenerator 'gmg':     n_local_elem()=8
GeneratedMeshGenerator 'gmg':     n_active_elem()=16
GeneratedMeshGenerator 'gmg':   n_subdomains()=1
GeneratedMeshGenerator 'gmg':   n_partitions()=2
GeneratedMeshGenerator 'gmg':   n_processors()=2
GeneratedMeshGenerator 'gmg':   n_threads()=1
GeneratedMeshGenerator 'gmg':   processor_id()=0
GeneratedMeshGenerator 'gmg':   is_prepared()=true
GeneratedMeshGenerator 'gmg':   is_replicated()=true
GeneratedMeshGenerator 'gmg':
GeneratedMeshGenerator 'gmg':  Mesh Bounding Box:
GeneratedMeshGenerator 'gmg':   Minimum: (x,y,z)=(       0,        0,        0)
GeneratedMeshGenerator 'gmg':   Maximum: (x,y,z)=(       1,        1,        0)
GeneratedMeshGenerator 'gmg':   Delta:   (x,y,z)=(       1,        1,        0)
GeneratedMeshGenerator 'gmg':
GeneratedMeshGenerator 'gmg':  Mesh Element Type(s):
GeneratedMeshGenerator 'gmg':   QUAD4
GeneratedMeshGenerator 'gmg':
GeneratedMeshGenerator 'gmg':  Mesh Nodesets:
GeneratedMeshGenerator 'gmg':   None
GeneratedMeshGenerator 'gmg':
GeneratedMeshGenerator 'gmg':  Mesh Sidesets:
GeneratedMeshGenerator 'gmg':   Sideset 0 (bottom), 4 sides (EDGE2), 4 elems (QUAD4), 5 nodes
GeneratedMeshGenerator 'gmg':    Side volume: 1
GeneratedMeshGenerator 'gmg':    Bounding box minimum: (x,y,z)=(       0,        0,        0)
GeneratedMeshGenerator 'gmg':    Bounding box maximum: (x,y,z)=(       1,        0,        0)
GeneratedMeshGenerator 'gmg':    Bounding box delta: (x,y,z)=(       1,        0,        0)
GeneratedMeshGenerator 'gmg':   Sideset 1 (right), 4 sides (EDGE2), 4 elems (QUAD4), 5 nodes
GeneratedMeshGenerator 'gmg':    Side volume: 1
GeneratedMeshGenerator 'gmg':    Bounding box minimum: (x,y,z)=(       1,        0,        0)
GeneratedMeshGenerator 'gmg':    Bounding box maximum: (x,y,z)=(       1,        1,        0)
GeneratedMeshGenerator 'gmg':    Bounding box delta: (x,y,z)=(       0,        1,        0)
GeneratedMeshGenerator 'gmg':   Sideset 2 (top), 4 sides (EDGE2), 4 elems (QUAD4), 5 nodes
GeneratedMeshGenerator 'gmg':    Side volume: 1
GeneratedMeshGenerator 'gmg':    Bounding box minimum: (x,y,z)=(       0,        1,        0)
GeneratedMeshGenerator 'gmg':    Bounding box maximum: (x,y,z)=(       1,        1,        0)
GeneratedMeshGenerator 'gmg':    Bounding box delta: (x,y,z)=(       1,        0,        0)
GeneratedMeshGenerator 'gmg':   Sideset 3 (left), 4 sides (EDGE2), 4 elems (QUAD4), 5 nodes
GeneratedMeshGenerator 'gmg':    Side volume: 1
GeneratedMeshGenerator 'gmg':    Bounding box minimum: (x,y,z)=(       0,        0,        0)
GeneratedMeshGenerator 'gmg':    Bounding box maximum: (x,y,z)=(       0,        1,        0)
GeneratedMeshGenerator 'gmg':    Bounding box delta: (x,y,z)=(       0,        1,        0)
GeneratedMeshGenerator 'gmg':
GeneratedMeshGenerator 'gmg':  Mesh Edgesets:
GeneratedMeshGenerator 'gmg':   None
GeneratedMeshGenerator 'gmg':
GeneratedMeshGenerator 'gmg':  Mesh Subdomains:
GeneratedMeshGenerator 'gmg':   Subdomain 0: 16 elems (QUAD4, 16 active), 25 active nodes
GeneratedMeshGenerator 'gmg':    Volume: 1
GeneratedMeshGenerator 'gmg':    Bounding box minimum: (x,y,z)=(       0,        0,        0)
GeneratedMeshGenerator 'gmg':    Bounding box maximum: (x,y,z)=(       1,        1,        0)
GeneratedMeshGenerator 'gmg':    Bounding box delta: (x,y,z)=(       1,        1,        0)
GeneratedMeshGenerator 'gmg':   Global mesh volume = 1
```

The parameter [!param](/Mesh/GeneratedMeshGenerator/output) enables the output of a mesh file immediately after it is generated. It can be used to view the intermediate meshes block during mesh generation.

Take the simple example:

!listing test/tests/meshgenerators/output_intermediate_mesh/output_intermediate_mesh.i block=Mesh

The above will result in an intermediate mesh file 'left_in.e' in addition to the final mesh file 'output_intermediate_mesh_in.e' when ran in mesh only mode.
