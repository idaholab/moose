# MeshRepairGenerator

!syntax description /Mesh/MeshRepairGenerator

## Overview

The `MeshRepairGenerator` provides a collection of operations to repair defects / modify features in a mesh.
If the defects/features are not present in the mesh, the `MeshRepairGenerator` does not modify the mesh.
The operations currently implemented are:

- overlapping node merges. This operation considers all nodes in the entire mesh and looks for nodes in neighboring
  elements that may overlap. If overlaps are found, only one node is kept.

- flipping the orientation of negative volume elements. Negative volume elements can arise from a wrong orientation.

- separating elements in subdomains into several subdomains depending on their element types. Subdomains with a mix of
  element types are not supported by [Exodus.md] output. The new split subdomains' names have the type of the element appended to their respective names.

- merging boundaries with the same name but different boundary IDs.

- repairing degenerate (near-zero-quality) elements, gated by
  [!param](/Mesh/MeshRepairGenerator/fix_degenerate_elements). Degeneracy is classified into four kinds: a
  **zero-volume** element (thin in all dimensions), flagged by
  [!param](/Mesh/MeshRepairGenerator/zero_volume_fraction) (or [!param](/Mesh/MeshRepairGenerator/zero_area_fraction) in
  2D); a **sliver** (thin in two dimensions, e.g. a needle tetrahedron, or in 2D a thin triangle, quadrilateral, or
  polygon); a **pancake** (thin in one dimension, i.e. a flat or squashed element); and an element **collapsed to a
  lower topology** by one or more short edges or in-plane vertices (a quadrilateral degenerating into a triangle, a
  pyramid into a tetrahedron, or a hexahedron into a prism). The flatness (flap) test
  [!param](/Mesh/MeshRepairGenerator/flatness_tol) flags pancakes and slivers by shape; either the shape test or the
  measure test can be disabled by setting its tolerance to 0. The repairs currently implemented, by element type, are:

  - **zero-volume** (point-collapse) elements: a first-order element small in *every* dimension - its diameter (maximum
    vertex separation) below the isotropic length equivalent of
    [!param](/Mesh/MeshRepairGenerator/zero_volume_fraction) (3D) or
    [!param](/Mesh/MeshRepairGenerator/zero_area_fraction) (2D) - is removed by merging all of its vertices onto a
    single representative node (a sub-tolerance move) and deleting it. The merge is committed only if every neighbor
    stays non-degenerate and non-inverted; otherwise the element is left in place. This handles an isolated
    point-collapse; a point-collapse that shares a face or edge with another element (a degenerate cluster) is left in
    place, as is line-collapse (a sliver) and plane-collapse (a pancake), which the routines below handle instead.

  - first-order 2D elements (`TRI3`, `QUAD4`, polygons): a 2D **sliver** is flagged if its area is below
    [!param](/Mesh/MeshRepairGenerator/zero_area_fraction) times the mesh surface-area scale, or if every vertex other
    than the two ends of its longest edge lies within [!param](/Mesh/MeshRepairGenerator/flatness_tol) times the
    longest-edge length from that edge. Each sliver is removed and absorbed into the element sharing its longest edge,
    keeping the surface conformal (no holes or hanging nodes). A triangle sliver sharing a triangle neighbor splits that
    neighbor into two triangles, so an all-triangle mesh stays all-triangle. Otherwise the neighbor absorbs the sliver's
    remaining vertices into the shared edge and is promoted by its new vertex count: a triangle becomes a quadrilateral,
    and anything else becomes a polygon. If the longest edge is on a surface boundary (no neighbor to absorb the
    sliver), the sliver is left in place.

  - `TET4` elements: a tetrahedron is flagged if its volume is below
    [!param](/Mesh/MeshRepairGenerator/zero_volume_fraction) times the mesh bounding-box volume (a zero-volume or needle
    sliver), or if the vertex opposite its largest face lies within [!param](/Mesh/MeshRepairGenerator/flatness_tol)
    times sqrt(largest-face area) of that face (a flat pancake). Each is repaired by an **edge collapse**: one of its
    edges is collapsed (a node is merged onto another existing node), removing the element while keeping a valid,
    conformal, manifold mesh. A collapse is committed only if it does not invert or re-degenerate any neighboring
    tetrahedron, does not create a non-manifold configuration, and does not move/remove a boundary node (the mesh
    boundary is never distorted); the [!param](/Mesh/MeshRepairGenerator/tet_collapse_volume_floor) parameter sets the
    relative volume below which a reshaped neighbor is rejected. Elements with no admissible collapse (for example a flat
    pancake all of whose nodes are on the boundary, or one whose every collapse would invert a neighbor) are left in
    place and reported. Edge collapse (rather than polyhedron-based absorption) is used for tetrahedra because the union
    of a flat tet with a neighbor is non-convex, and libMesh polyhedra must be convex. Only first-order `TET4` elements
    are handled this way.

  - `PYRAMID5` elements: a pyramid is flagged if its volume is below
    [!param](/Mesh/MeshRepairGenerator/zero_volume_fraction) times the mesh bounding-box volume, or if its apex lies
    within [!param](/Mesh/MeshRepairGenerator/flatness_tol) times sqrt(base area) of its quad base (a flat pancake). A
    flat pancake pyramid is repaired by **absorbing it into the element across its quad base** (a hexahedron, prism,
    polyhedron, or another pyramid): the shared quad face is dissolved and the neighbor is replaced by a `C0Polyhedron`
    made of its remaining faces plus the pyramid's four triangular cap faces. No node is moved, so the surrounding
    elements stay conformal. Here the union *is* convex (a neighbor capped by a shallow pyramid lid), so polyhedron
    absorption is valid. The absorption is committed only if the apex projects inside the quad-base footprint and the
    resulting polyhedron is a sound cell (positive volume, invertible mapping); a pyramid with no element across its quad
    base, or for which the union would be invalid, is left in place and reported. A pyramid **sliver** - a needle thin in
    two dimensions, of negligible volume but *not* flat (its apex far from a degenerate base) - is instead removed by
    collapsing its shortest edge, so the surrounding elements meet, when that leaves every neighbor valid.

  - `PRISM6` (wedge) elements: a wedge is flagged if its volume is below
    [!param](/Mesh/MeshRepairGenerator/zero_volume_fraction) times the mesh bounding-box volume, or by the
    [!param](/Mesh/MeshRepairGenerator/flatness_tol) test, and is then classified and repaired one of two ways. A
    **flat (pancake)** wedge (its top triangle squashed onto the bottom) is repaired by **collapsing** the top triangle
    onto the bottom so the elements above and below it meet; the collapse is a sub-tolerance move (it cannot distort the
    boundary by more than the element's own thickness) and is committed only if the three quad sides are unshared and it
    inverts or degenerates no neighbor, otherwise the wedge is left in place. A **thin-cross-section (blade) sliver**
    wedge (whose triangular cross-section is itself a sliver) is repaired by **absorbing it into the element across its
    longest quad side** (the 3D analog of absorbing a 2D sliver triangle into its longest-edge neighbor), which becomes a
    `C0Polyhedron`. A wedge with no admissible repair is left in place and reported.

  - `HEX8` elements: a hexahedron is flagged if its volume is below
    [!param](/Mesh/MeshRepairGenerator/zero_volume_fraction) times the mesh bounding-box volume, or if one of its three
    pairs of opposite faces is separated by less than [!param](/Mesh/MeshRepairGenerator/flatness_tol) times
    sqrt(face area) (a flat slab **pancake**, thin in one axis direction). It is repaired by **collapsing that squashed
    pair of opposite faces together** so the elements on either side of the slab meet; like the wedge collapse this is a
    sub-tolerance move, committed only if the four connecting side faces are unshared and it inverts or degenerates no
    neighbor, otherwise the hexahedron is left in place. A **sliver** hexahedron thin in *two* dimensions (a
    needle/column, two of its three opposite-face-pair separations below
    [!param](/Mesh/MeshRepairGenerator/flatness_tol) times the largest) is instead collapsed onto its long axis - its
    two thin cross-sections are each merged to a point - removing it so the elements around it meet; this is committed
    only when it leaves every neighbor valid (mainly an isolated or boundary column), otherwise it is left in place and
    reported.

  - topology collapse - `QUAD4` -> `TRI3`: a quadrilateral collapsed to a triangle by a **short edge**
    (two adjacent vertices within [!param](/Mesh/MeshRepairGenerator/flatness_tol) times the opposite span) or a
    **colinear vertex** (a vertex lying on the segment between its two neighbors, "not sticking out") is reduced to a
    triangle by collapsing the redundant vertex onto a neighbor. Every element sharing the collapsed edge is retyped
    consistently; the collapse is committed only if each such element reduces to a valid type and no element is
    inverted, and a colinear vertex is only removed when it is redundant in every element that uses it (so no hanging
    node is created), otherwise the quadrilateral is left in place and reported. A `C0POLYGON` with a redundant
    vertex is likewise reduced to an `(n-1)`-sided polygon, but only when every element sharing that vertex is itself a
    polygon, so all reduce onto polygons together.

  - topology collapse - `PYRAMID5` -> `TET4`: a pyramid whose quad base has a redundant vertex (a short
    base edge or a colinear base vertex) is reduced to a tetrahedron by collapsing that base vertex, so the base
    becomes a triangle. As with the other collapses every element sharing the collapsed edge must reduce to a valid
    type, so a pyramid whose base is shared with an element that cannot reduce (for example a hexahedron) is left in
    place and reported.

  - topology collapse - `PRISM6` -> `PYRAMID5`: a wedge pinched at one corner (a short vertical edge, its top node
    within [!param](/Mesh/MeshRepairGenerator/flatness_tol) times the element diameter of the bottom node) is reduced
    to a pyramid by collapsing that vertical edge - the merged corner becomes the apex and the opposite lateral quad
    becomes the base - subject to the same co-edge-reducibility condition.

  - topology collapse - `HEX8` -> `PRISM6`: a hexahedron with a lateral face pinched to a vertical edge (both of that
    face's horizontal edges short) is reduced to a prism by collapsing those two edges together, so each squashed
    bottom/top face becomes a triangle. This is committed only when the pinch is local (no other element shares a
    collapsed edge and would have to change type), otherwise the hexahedron is left in place and reported.

- renumbering the nodes and elements to have a contiguous ordering.

- splitting non-convex polygons into convex polygons

!syntax parameters /Mesh/MeshRepairGenerator

!syntax inputs /Mesh/MeshRepairGenerator

!syntax children /Mesh/MeshRepairGenerator
