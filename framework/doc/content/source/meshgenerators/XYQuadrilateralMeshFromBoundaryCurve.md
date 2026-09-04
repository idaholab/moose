# XYQuadrilateralMeshFromBoundaryCurve

!syntax description /Mesh/XYQuadrilateralMeshFromBoundaryCurve

## Overview

We mesh a curved region with quadrilaterals using a chain of generators: a triangulation in the
domain defined by the curve that is biased
toward right angles, the conversion of that triangulation into quadrilaterals, the snap of the
boundary nodes the conversion added back onto the boundary curves, and an optional smoothing
pass. `XYQuadrilateralMeshFromBoundaryCurve` packages that chain behind one block. It creates the
generators as sub-generators:

1. [XYFrontalDelaunayGenerator.md] triangulates the region enclosed by
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/boundary), around the
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/holes), at the element size
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/desired_area). Its `metric` and `orientation` keep
   their defaults, which place the right isosceles triangles that recombine well.
2. [TriToQuadConverter.md] merges pairs of triangles that reach
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/eta_min). With
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/all_quad), its default, the triangles that could not
   be merged are eliminated and the mesh is purely quadrilateral; without it the mesh is
   quad-dominant, and the triangles of each subdomain are collected in a subdomain of their own
   named after it with the suffix `_tri`.
3. [MoveBoundaryNodesToCurveGenerator.md] snaps each boundary named in
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/snap_boundaries) onto the curve of the
   [ParsedCurveGenerator.md] it is paired with in
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/parsed_curve_generators), one snap per pair. Boundaries
   whose geometry is not a parametric curve need no pair and are left alone.
4. [SmoothMeshGenerator.md] runs a variational smoothing pass, unless
   [!param](/Mesh/XYQuadrilateralMeshFromBoundaryCurve/smooth) turns it off. The variational
   algorithm cannot tangle the mesh and only allows node movement that leaves the domain
   unchanged, so the snapped geometry survives the smoothing.

## Example Syntax

A gear profile whose bore is the same gear profile scaled down and rotated slightly, meshed with
quadrilaterals in one step. Both boundaries are snapped back onto their curves:

!listing test/tests/meshgenerators/xy_quad_mesh_from_boundary_curve/gear.i block=Mesh

!media large_media/framework/meshgenerators/xy_quad_mesh_from_boundary_curve_gear.png style=width:60%;margin-left:auto;margin-right:auto; id=fig:xy_quad_mesh_from_boundary_curve_gear caption=The gear mesh of the input above.

!syntax parameters /Mesh/XYQuadrilateralMeshFromBoundaryCurve

!syntax inputs /Mesh/XYQuadrilateralMeshFromBoundaryCurve

!syntax children /Mesh/XYQuadrilateralMeshFromBoundaryCurve
