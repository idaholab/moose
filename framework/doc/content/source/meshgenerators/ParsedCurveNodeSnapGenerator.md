# ParsedCurveNodeSnapGenerator

!syntax description /Mesh/ParsedCurveNodeSnapGenerator

## Overview

A boundary meshed from a curve is a chain of straight element edges, so it encloses the chord
polygon of the curve rather than the curve itself. Every node of that chain does lie on the
curve, but any node added to the boundary afterwards lies on a chord, and the gap between the
chord and the curve is real geometry that the mesh has lost.

`ParsedCurveNodeSnapGenerator` closes that gap. It takes the boundary named in
[!param](/Mesh/ParsedCurveNodeSnapGenerator/boundary) and moves each of its nodes to the closest
point of the curve defined by the [ParsedCurveGenerator.md] named in
[!param](/Mesh/ParsedCurveNodeSnapGenerator/curve_generator). The curve is not re-entered here:
[!param](/Mesh/ParsedCurveGenerator/section_bounding_t_values) and
[!param](/Mesh/ParsedCurveGenerator/is_closed_loop) are read from the parameters of that generator,
and the curve itself is evaluated by that generator, so the nodes are snapped onto the same curve
they were meshed from, and there is no second definition to keep in step.

The boundary may be given as a sideset, as a nodeset, or as both; the nodes of its sides and of
its nodeset entries are all collected. Since the curve is defined in the XY-plane, only the
in-plane coordinates of a node are changed. The input mesh must be replicated.

This is worth doing after any step that adds nodes to a curved boundary, and
[!param](/Mesh/TriToQuadGenerator/all_quad) is the case that motivates it: eliminating the
leftover triangles subdivides every element, and the boundary nodes it creates are the
midpoints of the existing chords. They sit strictly inside the curve until they are snapped.

In the circle example below, the boundary mesh approximates a unit circle by the chords of a
32-sided polygon and so encloses an area of $3.121445$, in error by $0.64\%$. Snapping brings the
enclosed area to $3.141032$, in error by $0.018\%$.

!media media/mesh/parsed_curve_node_snap.png style=width:90%;margin-left:auto;margin-right:auto; id=fig:parsed_curve_node_snap caption=A quadrant of that circle boundary, with the curve drawn in red. Before the snap, the boundary nodes the conversion added lie on the chords, inside the curve; the snap moves every boundary node onto the curve and the smoothing relaxes the elements behind it.

## Finding the Closest Point

The closest point is found in the curve parameter $t$, not in space. Each section of the curve
delimited by [!param](/Mesh/ParsedCurveGenerator/section_bounding_t_values) is first sampled
uniformly at
[!param](/Mesh/ParsedCurveNodeSnapGenerator/samples_per_section) values of $t$, and the sample
nearest the node brackets the minimum between its two neighbors. A golden-section search then
refines $t$ within that bracket.

The sampling is what makes the bracket correct, so
[!param](/Mesh/ParsedCurveNodeSnapGenerator/samples_per_section) has to resolve the features of
the curve: a curve that turns sharply, or approaches itself, within one sampling interval can
bracket the wrong minimum, and the refinement will then converge to a point that is close by but
not closest. Raising the parameter costs only setup time.

On a closed loop the parameter is periodic, and the search is too. The sample before the first
and the sample after the last are taken across the seam, one period below and above the sampled
range, so a node near the start of the curve is not held back by the end of the parameter
interval.

## Usage

One instance snaps one boundary onto one curve. A domain bounded by several curves needs one
instance per pair, chained through
[!param](/Mesh/ParsedCurveNodeSnapGenerator/input).

Place the snap after the quadrilateral conversion, so that it also catches the nodes that
conversion introduced, and follow it with a [SmoothMeshGenerator.md] to let the interior absorb
the boundary movement. The Laplace algorithm holds boundary nodes fixed, so the recovered
geometry is kept while the elements just inside it are relaxed.

[CircularBoundaryCorrectionGenerator.md] addresses a related but distinct problem. It corrects
the radius of a circular polygonal boundary so that the polygon encloses the area of the circle
it stands for, keeping the boundary polygonal. `ParsedCurveNodeSnapGenerator` moves nodes onto an
arbitrary parametric curve, and reduces the polygonization error rather than compensating for it.

## Example Syntax

A unit circle triangulated by [XYFrontalDelaunayGenerator.md], converted to quadrilaterals,
snapped back onto the circle it was generated from, and smoothed:

!listing test/tests/meshgenerators/parsed_curve_node_snap_generator/snap_circle.i block=Mesh

The boundary the snap acts on is the one that
[!param](/Mesh/XYFrontalDelaunayGenerator/output_boundary) named on the triangulation, which is
the sideset the outer curve left behind.

!syntax parameters /Mesh/ParsedCurveNodeSnapGenerator

!syntax inputs /Mesh/ParsedCurveNodeSnapGenerator

!syntax children /Mesh/ParsedCurveNodeSnapGenerator
