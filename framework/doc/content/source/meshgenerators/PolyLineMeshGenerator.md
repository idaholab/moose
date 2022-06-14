# PolyLineMeshGenerator

!syntax description /Mesh/PolyLineMeshGenerator

Using the `PolyLineMeshGenerator` object from within the
[Mesh](/Mesh/index.md) block of the input file will construct an open
or closed (looped) one-dimensional manifold of Edge elements
connecting each adjacent pair of points in a user-specified list.
Points can live in 3-D space by default, but loops in the X-Y plane
are particularly useful as inputs to subsequent triangulator mesh
generators.

## Example

The following Mesh block will connect the specified 3 points with a
total of 9 Edge2 elements.

!listing test/tests/meshgenerators/polyline_mesh_generator/polyline_mesh_generator_loop_refine.i block=Mesh

!syntax parameters /Mesh/PolyLineMeshGenerator

!syntax inputs /Mesh/PolyLineMeshGenerator
