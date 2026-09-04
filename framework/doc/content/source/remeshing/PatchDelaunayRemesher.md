# PatchDelaunayRemesher

!syntax description /Remeshing/Remeshers/PatchDelaunayRemesher

## Description

A triangle fails when its [!param](/Remeshing/Remeshers/PatchDelaunayRemesher/quality_metric) lies
outside [!param](/Remeshing/Remeshers/PatchDelaunayRemesher/quality_lower_bound) and
[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/quality_upper_bound). Each failing triangle is
grown into a patch by [!param](/Remeshing/Remeshers/PatchDelaunayRemesher/n_layers) layers of
point-neighbors, patches that overlap merge, and the closed boundary loops of the merged patch are
extracted and retriangulated. Quality is not the only way to be selected: with
[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/sizing_variable) and
[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/coarsen_fraction) set, a triangle that has become
small against its target size is selected on top of the ones the metric condemns, which is the
coarsening path described in [#sizing].

The nodes of the patch boundary are pinned: the new triangles reuse those very node objects, so
exterior boundaries, sidesets and subdomain interfaces that run along a patch boundary come through
unchanged, and only the interior of the patch is new. Because the boundary is read off the mesh
rather than from an analytic description, an arbitrarily deformed interface or crack path is
conformed to as it stands.

A patch whose boundary passes through the same node twice is healed by absorbing every element
around that node, which moves the node into the patch interior. A patch whose boundary is still
degenerate in the deformed configuration — pinched, collapsed, or crossing itself — is rejected and
retried grown by another layer, up to three times, which moves its boundary outward onto healthier
elements. A patch that is still rejected after that is left alone for this step, which the console
reports; rejection is not an error, and a step in which every patch is rejected changes nothing.

Growth stops at any element that is not a `TRI3`, since the triangulator cannot reproduce one. The
mesh must be two-dimensional, and only `TRI3` elements are measured and replaced.

### Sizing the New Triangles id=sizing

With [!param](/Remeshing/Remeshers/PatchDelaunayRemesher/desired_area) left at zero and no sizing
field given, the target area is graded edge by edge off the length of the patch boundary edges.
Inheriting the size from the boundary rather than from the area of the elements being replaced makes
remeshing the same patch again a fixed point instead of a sequence that drifts finer with every
pass, and grading it is what suits a patch whose boundary spans more than one mesh size: the
boundary is pinned and cannot be subdivided, so a single target that disagreed with a boundary edge
would force slivers against it. Giving
[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/desired_area) a positive value applies that one
area to the whole patch instead.

[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/sizing_variable) names a `CONSTANT MONOMIAL`
auxiliary variable carrying one target element size per element, read per element over the patch. It
replaces the boundary-graded default rather than modulating it, so the new triangles follow a size
field that varies across the patch and the boundary edges no longer set the size. Only one of the
two may set the target, and a positive
[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/desired_area) alongside it is a setup error.
The target area handed to the triangulator is that of the equilateral triangle whose side is the
target size, which sizes the new triangles a little under the target: they land inside the dead
band described in [#dead_band] rather than above the size a refining remesher would select again.

The size field also drives a second selection path, which is why
[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/coarsen_fraction) has to be given with it. An
element whose diameter falls below that fraction of the target on it is selected as over refined and
retriangulated, alongside whatever
[!param](/Remeshing/Remeshers/PatchDelaunayRemesher/quality_metric) condemns. This is how the
remesher coarsens a region a refining remesher such as [TriSplitRemesher.md] left behind once the
feature that justified the refinement has moved on.

!listing test/tests/remeshing/refine_coarsen_cycle.i block=Remeshing

### Coarsening Against a Pinned Boundary id=rim

The patch boundary is pinned, so it keeps its spacing through the surgery: retriangulating a patch
whose own boundary edges are still short would reproduce the state that selected it. A selected
patch is therefore grown until no edge of its boundary is short against the target size the interior
element owning that edge carries.

Short means the same thing to a boundary edge as to an element, but not by the same number, because
an element is measured by its diameter and a boundary offers a single edge instead. The boundary
test uses [!param](/Remeshing/Remeshers/PatchDelaunayRemesher/coarsen_fraction) scaled by
$1/\sqrt{2}$, the shortest edge of a right isosceles triangle as a fraction of its diameter, so that
the bound applies to an edge rather than to a diameter. Measured against the unscaled fraction, the
shortest edge of a correctly sized element would look short and the patch would grow out of the over
refined region without ever stopping.

A patch that cannot reach that condition, because growth ran into a non-`TRI3` element, a partition
seam, or the domain boundary, is deferred whole for this event, and the console reports how many
were deferred. Deferral is not a loss: those elements are still over refined at the next event and
are selected again.

The domain boundary is the case with no way out, since there is no element beyond it to absorb. Any
refined element sitting on an exterior boundary pins its patch for good, whatever the fraction is
set to: an element selected as over refined has a diameter under the fraction of its target, and its
shortest edge is that diameter over $\sqrt{2}$, which is the edge threshold itself. The edge is measured
against the target the interior element carries, so relaxing the field there afterwards condemns the
edge rather than excusing it. Keep the target coarse in a band along every exterior boundary the
mesh should stay coarsenable near, so nothing refines there and no short exterior edge is ever
manufactured. Driving the target fine right up to the boundary is the natural mistake and produces
exactly the short exterior edges no later event can repair. The size field of the input above is
held at its coarse end within one element of the top and bottom of the domain for exactly this
reason:

!listing test/tests/remeshing/refine_coarsen_cycle.i block=AuxKernels/sizing

### Pairing With a Refining Remesher id=dead_band

Paired with [TriSplitRemesher.md] reading the same field, the two remeshers act on opposite sides
of a dead band. The splitter acts on an element larger than its target and this remesher on one
smaller than [!param](/Remeshing/Remeshers/PatchDelaunayRemesher/coarsen_fraction) of it, so an
element between the two comes to rest and the mesh does not alternate between a refined and a
coarsened state from one event to the next. Keep the fraction below one half. A split halves the
diameter of an element that was larger than its target, so every child stands above half of that
target, and a fraction above one half lets the splitter feed its own output to the coarsening within
a single event. A run in which the two fight shows the remesh count advancing by two on every event
while the element sizes of the coarsened region oscillate.

The one fraction does three jobs, two measured in diameters and one in edges: it selects the over
refined elements, it sets the width of the dead band above them, and, scaled by $1/\sqrt{2}$ as
described in [#rim], it sets the shortest edge a patch boundary may carry before the patch is
deferred. Selecting fewer elements, a wider dead band and a more permissive edge test all want
it small; only prompt coarsening wants it large. The bound of one half governs the two diameter
jobs; the edge threshold is derived from the fraction rather than capped.

### Solution Transfer id=transfer

Every new triangle takes its values from the old element of the patch that contains its centroid,
located while the replaced elements are still in the mesh, and it inherits the subdomain of that
element rather than one subdomain for the whole patch. A patch that straddles a subdomain seam
therefore leaves each subdomain holding exactly the new triangles that lie inside it, and patch
growth pays no attention to subdomain ids, so patches do straddle seams routinely. Every new node
takes its values from the old element that contains it in the same way.

The old field is evaluated at one point for every degree of freedom to fill, so a retriangulated
patch carries the old solution sampled at the new nodes rather than integrated over them, and
sampling loses whatever the old field did between those points. Unlike a child of
[TriSplitRemesher.md], a new triangle here lies inside no single old triangle, so the transfer is
not exact and the integral of a variable moves by that resampling error across the event. The
error is small where the field is smooth, which is where a size field asks for coarsening.

### Choosing the Quality Bounds id=bounds

Left unset, both bounds fall back to the range libMesh suggests for the metric on a triangle. Which
value suits [!param](/Remeshing/Remeshers/PatchDelaunayRemesher/quality_lower_bound) depends on
whether the remesher's own output has to clear it.

Where it does, keep the bound below the quality the triangulator itself produces, which is roughly
0.4 to 0.5 for the default `SHAPE` metric. This is the sustained adaptive case, in which a patch
that stays selected is retriangulated again at every event: a bound above that mark condemns the
fresh triangles as well, so every patch grows into its neighbors and the replacement goes domain-wide
on the first event.

Where something other than this bound paces the remeshing, a bound the fresh triangles cannot clear
is a deliberate setting, and the ALE inputs on these pages use `quality_lower_bound = 0.9` for it.
There a [MeshMotionCriterion.md] decides when an event happens, or an
[ElementQualityCriterion.md] measuring a different metric does; the bound then only decides how much
of the mesh that event rebuilds. At 0.9 every patch the motion distorted requalifies, freshly
triangulated ones included, which is what makes the event rebuild the whole distorted region rather
than pick at it.

### Parallel Execution id=parallel

On a replicated mesh every rank measures the whole mesh, selects the failing elements in order of
id, and performs the identical surgery, so the copies stay in step without communication.

On a distributed mesh a rank measures and replaces only the elements it owns. Patch growth stops at
an element of another rank, which pins the nodes of the partition seam and confines each rank's
surgery to its own elements. The new entities are numbered out of a block of ids the rank carves
above the global maximum, so no rank has to ask another which ids it used, and the ids every rank
deleted are then handed to every other rank so the stale ghost copies are dropped. Cutting the
patches at the seams gives a distributed run a different, equally valid mesh from the one a
replicated run reaches from the same input. The remesher itself needs no extra input for this; the
mesh is what declares that it is distributed.

!listing test/tests/remeshing/distributed_remesh.i block=Mesh

## Example Input File Syntax

!listing test/tests/remeshing/ale_remesh_reset.i block=Remeshing

!syntax parameters /Remeshing/Remeshers/PatchDelaunayRemesher

!syntax inputs /Remeshing/Remeshers/PatchDelaunayRemesher

!syntax children /Remeshing/Remeshers/PatchDelaunayRemesher
