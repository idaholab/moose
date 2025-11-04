# BoundaryLayerSubdomainGenerator

!syntax description /Mesh/BoundaryLayerSubdomainGenerator

By default, the `BoundaryLayerSubdomainGenerator` only creates boundary layers on the internal side of a sideset.
It can also create boundaries along nodesets by setting the
[!param](/Mesh/BoundaryLayerSubdomainGenerator/include_nodesets), but note that internal nodesets will generate
a boundary layer on both sides.

!syntax parameters /Mesh/BoundaryLayerSubdomainGenerator

!syntax inputs /Mesh/BoundaryLayerSubdomainGenerator

!syntax children /Mesh/BoundaryLayerSubdomainGenerator
