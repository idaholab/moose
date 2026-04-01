# OrientSurfaceMeshGenerator

!syntax description /Mesh/OrientSurfaceMeshGenerator

It may occur in surface meshes that a surface element's orientation is flipped because the orientations
were not defined carefully. This mesh generator compares the orientation of the elements in the
[!param](/Mesh/OrientSurfaceMeshGenerator/included_subdomains). If it is opposite the normal/orientation specified
in the [!param](/Mesh/OrientSurfaceMeshGenerator/normal_to_align_with) parameter, e.g. if the dot product
of the two is negative, then the ordering of the nodes in the surface element is changed to flip its normal/orientation.

There are two options for using this mesh generator:

- using a input normal/orientation to re-align a mesh or elements from [!param](/Mesh/OrientSurfaceMeshGenerator/included_subdomains)
  directly.
- "flooding"/painting/propagating an orientation from selected element IDs onto their neighbors, by adjusting the 'flooding'
  and other parameters to carefully cover the desired portions of the mesh and changing their normals. An input normal/orientation
  can still be used though these element's orientation should likely suffice. This option uses the flood algorithm defined in the [SurfaceMeshGeneratorBase.md] base class.


!syntax parameters /Mesh/OrientSurfaceMeshGenerator

!syntax inputs /Mesh/OrientSurfaceMeshGenerator

!syntax children /Mesh/OrientSurfaceMeshGenerator
