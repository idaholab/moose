# TriToQuadGenerator

!syntax description /Mesh/TriToQuadGenerator

## Overview

The input mesh must consist exclusively of TRI3 elements, must lie in the XY plane, and must be
replicated. Meshes of higher-order triangles, and meshes that mix triangles with other element
types, are rejected.

Two conversion algorithms are available through
[!param](/Mesh/TriToQuadGenerator/algorithm), and they trade a guaranteed pure-quadrilateral
result against element quality.

!media media/mesh/tri_to_quad_algorithms.png style=width:90%;margin-left:auto;margin-right:auto; id=fig:tri_to_quad_algorithms caption=The two algorithms applied to the same frontal triangulation of a disk. `SUBDIVISION` splits every triangle into three quadrilaterals. `RECOMBINE` merges pairs of adjacent triangles and leaves the triangles that found no partner, shown in orange.

### Subdivision

`SUBDIVISION` splits every triangle into three quadrilaterals, each built on the triangle
centroid, one vertex, and the midpoints of the two edges meeting at that vertex. Every element of
the output is a QUAD4, the output is conformal, and the result does not depend on the triangle
quality of the input. A mesh of $m$ triangles becomes a mesh of $3m$ quadrilaterals at roughly
half the original element size.

The cost of that guarantee is quality: the centroid of each original triangle becomes a node of
valence three, so the output carries one irregular vertex per input triangle.

### Recombination

`RECOMBINE` merges pairs of adjacent triangles into quadrilaterals by deleting the edge they
share. Each pair scores a quality $\eta$ [!citep](remacle2012blossomquad), computed from the
internal angles $\alpha_k$ of the quadrilateral the merge would produce:

\begin{equation}
\eta = \max\left(0,\; 1 - \frac{2}{\pi} \max_k \left| \frac{\pi}{2} - \alpha_k \right| \right).
\end{equation}

A rectangle scores $\eta = 1$, and a pair whose merged shape is not convex scores $\eta = 0$.
Only pairs reaching [!param](/Mesh/TriToQuadGenerator/eta_min) are merged. Raising the threshold
buys quality at the cost of yield; lowering it leaves fewer triangles behind but admits
flatter quadrilaterals.

The pairing is greedy: the candidates are taken in order of decreasing $\eta$, keeping each
one whose two triangles are both still unmerged. This is fast but not optimal: a locally
attractive merge can consume a triangle that a better global pairing needed.

Recombination is quad-dominant, not pure quad. Triangles are left over wherever no admissible
partner remains.

Two kinds of candidate pair are never merged, regardless of their score:

- a pair whose shared edge carries any boundary id. Merging deletes that edge and would delete
  the sideset entry on it, so interior sidesets survive the conversion instead of being silently
  dropped.
- a pair whose two triangles belong to different subdomains. Merging across a subdomain
  interface would move that interface.

Where the two triangles of a merged pair disagree on an extra element integer, the merged
quadrilateral inherits the value of the +lower-id+ parent triangle. That rule makes the result
reproducible from one run to the next; it does not attempt to reconcile the two values.

### Leftover Triangles

The triangles that recombination could not merge can be handled in either of two ways, which are
mutually exclusive.

[!param](/Mesh/TriToQuadGenerator/tri_subdomain_name) moves them into a subdomain of their own,
leaving a mixed TRI3/QUAD4 mesh. Isolating them makes the recombination yield measurable, and
lets downstream physics be block-restricted away from them.

[!param](/Mesh/TriToQuadGenerator/all_quad) instead eliminates them, so that the output consists
exclusively of QUAD4 elements. After the merges are made, +every+ element of the mesh is
subdivided once: each merged quadrilateral into four, each leftover triangle into three. A mesh
whose recombination produced $n$ quadrilaterals and left $m$ triangles therefore ends with
$4n + 3m$ elements, at half the element size the merges produced.

Subdividing only the leftover triangles would be cheaper, but it would leave each of their
unsplit neighbors with a hanging node in the middle of a side. Splitting everything is what keeps
the mesh conformal, and it is why the two algorithms cannot be mixed element by element.

## Preserved Mesh Data

The conversion carries the following through, under both algorithms:

- subdomain ids and names;
- sideset ids and names. Where a boundary edge is split, under `SUBDIVISION` or under
  [!param](/Mesh/TriToQuadGenerator/all_quad), both halves inherit the ids of the original edge;
- nodesets, rebuilt on the converted mesh;
- extra element integers.

## Example Syntax

Splitting every triangle of a 32-triangle mesh into three quadrilaterals gives 96 QUAD4 elements
and no leftover triangle:

!listing test/tests/meshgenerators/tri_to_quad_generator/subdivision_all_quad.i block=Mesh

Recombining the same kind of mesh, with the leftover triangles collected into their own
subdomain. The mesh reaching the conversion carries two subdomains, a sideset on their interface,
a second sideset on an interior edge, a nodeset, and an extra element integer, none of which the
conversion discards:

!listing test/tests/meshgenerators/tri_to_quad_generator/recombine_ids.i block=Mesh

Running the same mesh through the same merges with
[!param](/Mesh/TriToQuadGenerator/all_quad) instead removes every remaining triangle:

!listing test/tests/meshgenerators/tri_to_quad_generator/all_quad_ids.i block=Mesh/to_quad

All three examples disable renumbering. The triangulation, the subdivision and the merges all
break ties by element id, so their results are reproducible only while the element numbering is
stable.

Triangulations intended for recombination are best produced by
[XYFrontalDelaunayGenerator.md], which biases the triangles toward right angles so that more
pairs clear [!param](/Mesh/TriToQuadGenerator/eta_min). When the boundary of the input is a
parametric curve, [ParsedCurveNodeSnapGenerator.md] can move the boundary nodes that
[!param](/Mesh/TriToQuadGenerator/all_quad) created back onto that curve.

!bibtex bibliography

!syntax parameters /Mesh/TriToQuadGenerator

!syntax inputs /Mesh/TriToQuadGenerator

!syntax children /Mesh/TriToQuadGenerator
