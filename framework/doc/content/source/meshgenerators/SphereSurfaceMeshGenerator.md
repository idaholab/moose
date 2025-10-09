# SphereSurfaceMeshGenerator

!syntax description /Mesh/SphereSurfaceMeshGenerator

This object generates a 2D mesh approximating the surface of a sphere in 3D space
using TRI3 triangle elements. Both center and radius of the sphere may be specified.

!media phase_field/sphere.gif style=width:30%;margin-left:20px;float:right;
       caption=Targeted mesh refinement prior to nucleus insertion.

The mesh is constructed by iterative refinement of an initial icosahedron (depth 0)
for a number of steps specified using the `depth` parameter. After each refinement step
all nodes are snapped to the sphere surface defined by `radius` and `center`.
Each refinement step multiplies the number of mesh elements by a factor of four.

!syntax parameters /Mesh/SphereSurfaceMeshGenerator

!syntax inputs /Mesh/SphereSurfaceMeshGenerator

!syntax children /Mesh/SphereSurfaceMeshGenerator

!bibtex bibliography
