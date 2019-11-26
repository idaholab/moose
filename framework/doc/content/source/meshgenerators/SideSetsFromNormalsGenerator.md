# SideSetsFromNormalsGenerator

This MeshGenerator adds a sideset to the mesh on every element face with the specified normal. If the mesh contains multiple disjoint faces with the same normal, they will all be added to the sideset.

Setting `fixed_normal = false` and changing the `variance` parameter allows the normal to vary in order to paint around curves. This feature is only supported with ReplicatedMesh.

See also [AllSideSetsByNormalsGenerator](/AllSideSetsByNormalsGenerator.md)

!syntax description /MeshGenerators/SideSetsFromNormalsGenerator

!syntax parameters /MeshGenerators/SideSetsFromNormalsGenerator

!syntax inputs /MeshGenerators/SideSetsFromNormalsGenerator

!syntax children /MeshGenerators/SideSetsFromNormalsGenerator
