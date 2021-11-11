# MeshGeneratorMesh

!syntax description /Mesh/MeshGeneratorMesh

The `MeshGeneratorMesh` is a `MooseMesh` object on which the mesh generators will build. They may
not act directly on this mesh, but eventually, after possibly multiple mesh generators were chained
together, their contribution should be merged into this mesh.

!alert note
The [!param](/Mesh/MeshGeneratorMesh/final_generator) is used when there is an ambiguity as to which
constructed mesh should be used by the simulation. A warning is output if this parameter is not provided
when an ambiguity is detected. However, in most cases, this parameter is not required and the warning
is really indicating an issue with the chaining of mesh generators.

!syntax parameters /Mesh/MeshGeneratorMesh

!syntax inputs /Mesh/MeshGeneratorMesh

!syntax children /Mesh/MeshGeneratorMesh
