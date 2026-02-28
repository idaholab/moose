# OrientSurfaceMeshGenerator

!syntax description /Mesh/OrientSurfaceMeshGenerator

It may occur in surface meshes that a surface element's orientation is flipped because the orientations
were not defined carefully. This mesh generator compares the orientation of the elements in the
[!param](/Mesh/OrientSurfaceMeshGenerator/included_subdomains). If it is opposite the normal specified
in the [!param](/Mesh/OrientSurfaceMeshGenerator/normal_to_align_with) parameter, e.g. if the dot product
of the two is negative, then the ordering of the nodes in the surface element is changed to flip its normal.

!syntax parameters /Mesh/OrientSurfaceMeshGenerator

!syntax inputs /Mesh/OrientSurfaceMeshGenerator

!syntax children /Mesh/OrientSurfaceMeshGenerator
