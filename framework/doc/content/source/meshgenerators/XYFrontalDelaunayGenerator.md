# XYFrontalDelaunayGenerator

!syntax description /Mesh/XYFrontalDelaunayGenerator

## Overview

`XYFrontalDelaunayGenerator` triangulates the planar region enclosed by an input boundary mesh,
optionally around holes, in the same way and with the same parameters as
[XYDelaunayGenerator.md]. It defines the region with
[!param](/Mesh/XYFrontalDelaunayGenerator/boundary) and
[!param](/Mesh/XYFrontalDelaunayGenerator/holes), limits the element size with
[!param](/Mesh/XYFrontalDelaunayGenerator/desired_area),
[!param](/Mesh/XYFrontalDelaunayGenerator/desired_area_func) or
[!param](/Mesh/XYFrontalDelaunayGenerator/use_auto_area_func), and names the result with
[!param](/Mesh/XYFrontalDelaunayGenerator/output_subdomain_name),
[!param](/Mesh/XYFrontalDelaunayGenerator/output_boundary) and
[!param](/Mesh/XYFrontalDelaunayGenerator/hole_boundaries). Consult that page for the meaning of
those parameters, for how a boundary ring is read out of an input mesh, and for hole stitching.

What differs is how the interior points are chosen. Rather than refining a triangulation until
every element meets the area limit, this generator advances a front
[!citep](rebay1993frontaldelaunay). A triangle counts as too large when its circumradius exceeds
the circumradius targeted at its centroid, the edges those triangles share with the acceptable
ones form the front, and the front is advanced by placing one point at the target size ahead of
an edge and inserting it with the Bowyer-Watson algorithm. Placing points at the size the mesh is
meant to have, instead of subdividing until that size is reached, gives a triangulation whose
element shapes are chosen rather than inherited.

The purpose of that control here is to feed [TriToQuadGenerator.md]. Recombination merges pairs
of adjacent triangles and scores the merge on how close the resulting internal angles are to
$\pi/2$ [!citep](remacle2012blossomquad), so a triangulation biased toward right angles yields
far more merges than an equilateral one.

The output consists of first-order TRI3 elements. The boundary input must also be first order,
and the generator requires a replicated mesh.

## Target Size Metric

[!param](/Mesh/XYFrontalDelaunayGenerator/metric) selects the norm in which the target size ahead
of the front is measured.

`L2` measures the distance to the new point in the Euclidean norm, which is the conventional
frontal-Delaunay choice and places points that make the triangles equilateral. It is the right
choice when the triangles are the final mesh.

`LINF` measures it in the $L^\infty$ norm of a local frame [!citep](remacle2013frontalquad).
Because the unit ball of that norm is a square rather than a circle, the placement favors
triangles that are right isosceles in the frame, and two such triangles sharing their hypotenuse
recombine into a near-square quadrilateral. This is the metric to use when
[TriToQuadGenerator.md] follows.

## Local Frame Orientation

The $L^\infty$ norm is only defined once a local frame is fixed, and
[!param](/Mesh/XYFrontalDelaunayGenerator/orientation) supplies it. The parameter has no effect
when [!param](/Mesh/XYFrontalDelaunayGenerator/metric) is `L2`, which needs no frame.

`BOUNDARY` takes the frame from the tangent of the nearest boundary segment. It requires no
solve, and it aligns the elements with the boundary where they meet it, but far from the boundary
the nearest segment is a poor guide and the frame can turn abruptly between neighboring points.

`CROSS_FIELD` instead solves for a smooth four-fold direction field over the domain. A coarse
background triangulation is built first; the boundary tangents are imposed on it in a
representation under which four directions $\pi/2$ apart are the same value, that representation
is smoothed by a Laplace solve, and the frame at any point is then interpolated from the
background mesh. The field is boundary-aligned near the boundary and varies smoothly inside,
which is what keeps the quadrilaterals aligned with each other across the interior.

A cross field cannot be smooth everywhere on every domain. It has singularities, and around a
singularity the quadrilateral mesh acquires a node whose valence is not four. Those singularities
are forced by the geometry, not by the solve: they arise where a boundary corner turns through an
angle that is +not+ a multiple of $\pi/2$, and where the tangent of a curved boundary winds far
enough that no smooth interpolation of the imposed directions exists in the interior. A circular
boundary is the clearest case of the second kind.

A reentrant corner is not by itself a source of one. The $270^\circ$ corner of a rectilinear
L-shaped domain turns through $3\pi/2$, a multiple of $\pi/2$, so the field passes through it
without a defect and no irregular vertex is required: three quadrilaterals meeting at that corner
is the regular configuration for it.

## Example Syntax

Triangulating a disk. This input sets neither
[!param](/Mesh/XYFrontalDelaunayGenerator/metric) nor
[!param](/Mesh/XYFrontalDelaunayGenerator/orientation), so the elements come out right isosceles
in the frame of a cross field solved over the disk:

!listing test/tests/meshgenerators/xy_frontal_delaunay_generator/frontal_circle.i block=Mesh

A boundary containing a hole. The front advances from the hole boundary as well as from the outer
boundary, and the name given in
[!param](/Mesh/XYFrontalDelaunayGenerator/hole_boundaries) is carried onto the sideset that the
hole leaves behind:

!listing test/tests/meshgenerators/xy_frontal_delaunay_generator/frontal_annulus.i block=Mesh

A polyline boundary with a reentrant corner, which the front reaches from two sides at once:

!listing test/tests/meshgenerators/xy_frontal_delaunay_generator/frontal_l_shape.i block=Mesh

The end-to-end use is the pipeline below, which triangulates an MBB-beam domain with three
circular holes and then recombines the result into quadrilaterals, collecting the triangles that
could not be merged into their own subdomain so the yield can be measured:

!listing test/tests/meshgenerators/mbb_pipeline/mbb_pipeline.i block=Mesh

!bibtex bibliography

!syntax parameters /Mesh/XYFrontalDelaunayGenerator

!syntax inputs /Mesh/XYFrontalDelaunayGenerator

!syntax children /Mesh/XYFrontalDelaunayGenerator
