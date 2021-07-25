# SideSetsFromNormalsGenerator

!syntax description /Mesh/SideSetsFromNormalsGenerator

See also: [AllSideSetsByNormalsGenerator](/AllSideSetsByNormalsGenerator.md)

!alert note
If the mesh contains multiple disjoint faces with the same normal, they will all be added to the sideset.

!alert note
This will generate internal (within a block) sidesets as well if there are internal faces with the desired normal.
For external (on the boundary of a block) only sidesets, use [SideSetsAroundSubdomainGenerator](/SideSetsAroundSubdomainGenerator.md)
with a [!param](/MeshGenerators/SideSetsAroundSubdomainGenerator/normal) parameter.

!syntax parameters /Mesh/SideSetsFromNormalsGenerator

!syntax inputs /Mesh/SideSetsFromNormalsGenerator

!syntax children /Mesh/SideSetsFromNormalsGenerator
