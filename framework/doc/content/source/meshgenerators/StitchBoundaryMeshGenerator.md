# StitchBoundaryMeshGenerator

!syntax description /Mesh/StitchBoundaryMeshGenerator

## Description

`StitchBoundaryMeshGenerator` stitches two boundaries by merging their nodes together. The boundary ids are specified by `stitch_boundaries_pair`. To fully stitch two boundaries, the pairs of nodes on two boundaries must be at the same locations within a default tolerance. The two boundaries must be in the same mesh.


## Example Input Syntax

!listing test/tests/meshgenerators/stitch_boundary_mesh_generator/stitch_2d.i block=Mesh

!syntax parameters /Mesh/StitchBoundaryMeshGenerator

!syntax inputs /Mesh/StitchBoundaryMeshGenerator

!syntax children /Mesh/StitchBoundaryMeshGenerator
