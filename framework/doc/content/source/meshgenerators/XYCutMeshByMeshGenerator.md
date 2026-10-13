# XYCutMeshByMeshGenerator

!syntax description /Mesh/XYCutMeshByMeshGenerator

## Overview

The `XYCutMeshByMeshGenerator` cuts a 2D primary mesh using the outer boundary of another 2D mesh
as the cut curve. The cutter mesh's outer boundary must form a single, simple, closed polyline.
This generalizes [`XYMeshLineCutter`](/XYMeshLineCutter.md) from a single-line cut
($ax+by+c=0$) to an arbitrary closed polyline defined by a second mesh.

The cut produces `C0POLYGON` elements where the cutter polyline crosses primary mesh elements,
mirroring the `CUT_ELEM_POLY` mode of `XYMeshLineCutter`. When a cutter polyline vertex lies
strictly inside a crossed primary element, that vertex is inserted as a vertex of the output
polygon on the cut edge — so the cut conforms to the cutter geometry exactly (no chord
approximation). Non-convex retained polygons that result from this insertion are automatically
split into convex pieces, since `C0POLYGON` works best on convex shapes.

## Modes

The [!param](/Mesh/XYCutMeshByMeshGenerator/mode) parameter controls which side(s) of the cutter
polyline are kept:

- `REMOVE_INSIDE` (default): the portion of the primary mesh inside the cutter polyline is
  deleted. The new cut interface receives `new_boundary_id`.
- `REMOVE_OUTSIDE`: only the inside of the cutter is kept.
- `KEEP_BOTH`: the primary mesh is split into two labeled subdomains along the cut, nothing is
  deleted. Inside and outside subdomains receive distinct name suffixes (defaulting to
  `_inside` and `_outside`).

The [!param](/Mesh/XYCutMeshByMeshGenerator/cutting_type) parameter selects the output element
type for crossed elements:

- `CUT_ELEM_POLY` (default): emits `C0POLYGON` elements for the clipped pieces. Bystander
  primary elements keep their original type (QUAD4/TRI3). Use this mode if you want to
  post-process polygons (e.g., chain
  [`ElementsToSimplicesConverter`](/ElementsToSimplicesConverter.md)).
- `CUT_ELEM_TRI`: in addition to clipping, triangulates the resulting polygons (and, for
  `KEEP_BOTH`, also splits remaining QUAD4 elements in the new subdomains so each kept
  subdomain has a single element type).

## Node snapping

Setting [!param](/Mesh/XYCutMeshByMeshGenerator/snap_tol) to a positive value enables a snap
pre-pass: primary mesh nodes within `snap_tol` of any cutter polyline vertex or edge are
projected onto the closest cutter feature before clipping. This eliminates sliver polygons
where the cutter nearly aligns with primary edges/nodes. By default, only interior primary
nodes are eligible for snapping (see
[!param](/Mesh/XYCutMeshByMeshGenerator/snap_only_interior_nodes)), so the primary mesh's
external boundary shape is preserved.

## Restrictions

Both the primary mesh and the cutter mesh must be 2D, in the XY plane, and use a
`ReplicatedMesh` (this is the default for most mesh generators). The cutter mesh's outer
boundary must be a single simple closed loop (no holes, no multiple disjoint regions).

The cut within each primary element must be expressible as a single chord (with optional
cutter polyline vertices inserted on the cut edge). If the cutter polyline winds within a
single primary element — for example, if the cutter is entirely contained inside one primary
element, or the cutter polyline dips into and out of the same primary edge — the generator
errors with a refinement hint. Refine the primary mesh near the cutter, or simplify the
cutter, to resolve.

## Example Syntax

Cut a unit square primary mesh by a smaller square cutter, removing the inside, and
triangulating the result for direct Exodus output:

!listing test/tests/meshgenerators/xy_cut_mesh_by_mesh/convex_cut.i

To preserve the polygon output (e.g., for further post-processing), use
`cutting_type = CUT_ELEM_POLY` and chain `ElementsToSimplicesConverter`:

!listing test/tests/meshgenerators/xy_cut_mesh_by_mesh/convex_cut_poly.i

To enable node snapping when the cutter does not precisely align with the primary grid:

!listing test/tests/meshgenerators/xy_cut_mesh_by_mesh/snap.i

!syntax parameters /Mesh/XYCutMeshByMeshGenerator

!syntax inputs /Mesh/XYCutMeshByMeshGenerator

!syntax children /Mesh/XYCutMeshByMeshGenerator
