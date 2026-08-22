# QuadMeshPipelineGenerator

!syntax description /Mesh/QuadMeshPipelineGenerator

## Overview

Meshing a curved region with quadrilaterals takes four chained generators: a triangulation biased
toward right angles, the conversion of that triangulation into quadrilaterals, the snap of the
boundary nodes the conversion added back onto the boundary curves, and a smoothing pass.
`QuadMeshPipelineGenerator` packages that chain behind one block. It creates the four generators
as sub-generators:

1. [XYFrontalDelaunayGenerator.md] triangulates the region enclosed by
   [!param](/Mesh/QuadMeshPipelineGenerator/boundary), around the
   [!param](/Mesh/QuadMeshPipelineGenerator/holes), at the element size
   [!param](/Mesh/QuadMeshPipelineGenerator/desired_area). Its `metric` and `orientation` keep
   their defaults, which place the right isosceles triangles that recombine well.
2. [TriToQuadConverter.md] merges pairs of triangles that reach
   [!param](/Mesh/QuadMeshPipelineGenerator/eta_min). With
   [!param](/Mesh/QuadMeshPipelineGenerator/all_quad), its default, the triangles that could not
   be merged are eliminated and the mesh is purely quadrilateral; without it the mesh is
   quad-dominant.
3. [MoveNodesToCurveGenerator.md] snaps each boundary named in
   [!param](/Mesh/QuadMeshPipelineGenerator/snap_boundaries) onto the curve of the
   [ParsedCurveGenerator.md] it is paired with in
   [!param](/Mesh/QuadMeshPipelineGenerator/parsed_curve_generators), one snap per pair. Boundaries
   whose geometry is not a parametric curve need no pair and are left alone.
4. [SmoothMeshGenerator.md] runs
   [!param](/Mesh/QuadMeshPipelineGenerator/smooth_iterations) Laplace iterations. The Laplace
   algorithm holds boundary nodes fixed, so the snapped geometry survives the smoothing.

The individual generators stay available for a pipeline that needs options this generator does
not expose; this generator only removes the boilerplate of the common case.

## Example Syntax

A gear profile whose bore is the same gear profile scaled down and rotated slightly, meshed with
quadrilaterals in one step. Both boundaries are snapped back onto their curves:

!listing test/tests/meshgenerators/quad_mesh_pipeline/gear.i block=Mesh

!media media/mesh/quad_mesh_pipeline_gear.png style=width:60%;margin-left:auto;margin-right:auto; id=fig:quad_mesh_pipeline_gear caption=The gear mesh of the input above.

!syntax parameters /Mesh/QuadMeshPipelineGenerator

!syntax inputs /Mesh/QuadMeshPipelineGenerator

!syntax children /Mesh/QuadMeshPipelineGenerator
