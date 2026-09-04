# CoarsenSurfaceMeshAlongSidesetGenerator

!syntax description /Mesh/CoarsenSurfaceMeshAlongSidesetGenerator

## Overview

This mesh generator coarsens a mesh of 2D elements (TRI3/QUAD4, with any orientation in 3D space)
along a sideset, by collapsing every other node lying on the sideset. Collapsing a node merges the
pair of elements it was shared between into a single, larger element, while preserving the sideset
curve itself.

For an example with triangles, consider three consecutive sideset nodes `A`, `B` and `C`, with two
triangles `(A, B, P)` and `(B, C, P)` connecting them. Collapsing the middle node `B` deletes the
triangle holding the side `A-B` and re-points the other triangle, leaving the single triangle
`(A, C, P)` and dropping node `B`.

The sideset may be **internal**, with elements on both sides of it. In that case the merge is
performed on the elements of both sides simultaneously, and the sideset is preserved (it now spans
the merged element sides).

The sidesets to coarsen along are selected either explicitly with
[!param](/Mesh/CoarsenSurfaceMeshAlongSidesetGenerator/boundaries), or by coarsening along all the sidesets
of the mesh except those listed in
[!param](/Mesh/CoarsenSurfaceMeshAlongSidesetGenerator/exclude_boundaries). Exactly one of the two must be
provided.

A single invocation removes all non-adjacent sideset nodes that meet the criteria outlined below, coarsening the sideset
discretization by roughly a factor of two. To coarsen further, either apply the generator multiple
times in sequence, or set
[!param](/Mesh/CoarsenSurfaceMeshAlongSidesetGenerator/coarsen_more_than_two_elements), which repeats the
coarsening pass within a single invocation until no further collapse is possible. In that case more
than two elements can be merged into one, so it is usually combined with the merge criteria below to
bound the amount of coarsening.

A collapse is skipped if it would invert or flatten an element. The collapse can be further
restricted, in order to preserve geometric features, with:

- [!param](/Mesh/CoarsenSurfaceMeshAlongSidesetGenerator/max_normal_deviation): the maximum angle, in
  degrees, between the normals of the two elements being merged. This prevents merging across a
  curved region or a feature edge of the surface.
- [!param](/Mesh/CoarsenSurfaceMeshAlongSidesetGenerator/max_merged_side_length): the maximum length of the
  side created along the sideset by the merge.
- [!param](/Mesh/CoarsenSurfaceMeshAlongSidesetGenerator/max_merged_element_area): the maximum area of an
  element created by the merge.

!alert note
Only TRI3 and QUAD4 elements are supported, and the input mesh must not be distributed. A QUAD4 that
would be deleted by a collapse (one holding the collapsed side) blocks that collapse, since it would
require converting the quad to a triangle; quads that are only re-pointed are supported.

!syntax parameters /Mesh/CoarsenSurfaceMeshAlongSidesetGenerator

!syntax inputs /Mesh/CoarsenSurfaceMeshAlongSidesetGenerator

!syntax children /Mesh/CoarsenSurfaceMeshAlongSidesetGenerator
