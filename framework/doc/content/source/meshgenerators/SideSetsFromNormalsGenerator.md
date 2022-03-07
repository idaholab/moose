# SideSetsFromNormalsGenerator

!syntax description /Mesh/SideSetsFromNormalsGenerator

See also: [AllSideSetsByNormalsGenerator](/AllSideSetsByNormalsGenerator.md)

!alert note
If the mesh contains multiple disjoint faces with the same normal, they will all be added to the sideset.

!alert note
This will not generate internal (within the domain, on boundaries between blocks) sidesets,
even if there are internal faces with the desired normal.
For internal sidesets, use [SideSetsAroundSubdomainGenerator](/SideSetsAroundSubdomainGenerator.md)
with a [!param](/Mesh/SideSetsAroundSubdomainGenerator/normal) parameter.

!syntax parameters /Mesh/SideSetsFromNormalsGenerator

!syntax inputs /Mesh/SideSetsFromNormalsGenerator

!syntax children /Mesh/SideSetsFromNormalsGenerator
