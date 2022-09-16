# XYDelaunayGenerator

!syntax description /Mesh/XYDelaunayGenerator

## Examples

Using instances of the [PolyLineMeshGenerator.md] to create a boundary
and a few holes, followed by a `XYDelaunayGenerator` object to
triangulate the region between them, the [Mesh](/Mesh/index.md) block
shown in the input file snippet below generates the final mesh shown
in [Figure 1](#withholes).

For this example a specified fixed interpolation of boundary edges is
used, but refinement to a desired maximum triangle size allows
automatic placement of nodes in the mesh interior.

!listing test/tests/meshgenerators/xy_delaunay_generator/xydelaunay_with_holes.i block=Mesh

!media framework/meshgenerators/poly2tri_with_holes.png
      style=width:32%;
      id=withholes
      caption=Fig. 1: Resulting triangulated mesh from a polyline boundary and holes.

With the stitching options, meshes used as "holes" can be inserted
into those portions of the output mesh.  For this use case some care
may be required: refinement of stitched hole boundaries should be
disallowed so that the boundary nodes in the newly triangulated mesh
still precisely match the boundary nodes in the hole mesh.

In the input file snippet below, hole stitching is done recursively,
so that each internal "boundary" polyline (after refinement) remains
preserved in the final mesh shown in [Figure 2](#nested).

!listing test/tests/meshgenerators/xy_delaunay_generator/xydelaunay_nested.i block=Mesh

!media framework/meshgenerators/poly2tri_nested.png
      style=width:32%;
      id=nested
      caption=Fig. 2: Resulting triangulated mesh with nested polyline boundaries and an internal grid. 

!syntax parameters /Mesh/XYDelaunayGenerator

!syntax inputs /Mesh/XYDelaunayGenerator
