# TriSplitRemesher

!syntax description /Remeshing/Remeshers/TriSplitRemesher

## Description

A `TRI3` is selected when its longest edge is longer than the target size
[!param](/Remeshing/Remeshers/TriSplitRemesher/sizing_variable) holds on that element. Each selected
triangle is split into four by the midpoints of its three edges, which halves its edge lengths and
reproduces the shape of the parent in every child.

Splitting one triangle leaves a midpoint hanging on each neighbor that was not split, so the
neighbors are closed as well. A neighbor that acquired one midpoint is bisected from that midpoint
to its opposite vertex; a neighbor that acquired two is promoted to a four-way split of its own,
which puts a midpoint on its third side too. The mesh the event leaves is conforming and carries no
hanging node. The closure can cascade: a neighbor promoted to a four-way split puts a midpoint on
its own neighbors, which can promote them in turn, so one event may lay a layer of bisections well
beyond the elements the field selected, across a subdomain seam included. An exterior side has no
neighbor to demand a midpoint on it, so an element on the domain boundary is bisected rather than
quartered unless two of its sides are split at once. A bisection is never undone by this remesher.

A bisection is forced by conformity rather than chosen by size, so its children are not held to the
target: their size follows from the geometry of the parent and may fall below what the size field
asks for. Such a child can be selected on a later event and carry the refinement deeper still. This
is the intended behavior, the alternative being a hanging node, so the size field is a target for
the elements the remesher selects and not a floor the whole mesh respects.

The field must be a `CONSTANT MONOMIAL` auxiliary variable. A positive lower bound on the target is
what makes the splitting stop, since a target no element can reach would be refined against forever.
[!param](/Remeshing/Remeshers/TriSplitRemesher/min_element_size) holds the target at such a floor
directly: where the field asks for less, the floor takes over, and the field may fall to zero or
below there. Without the parameter the field itself must be positive on every element, typically by
clamping the expression that computes it, and the run ends with an error naming the element where it
is not. The same field convention is shared with [PatchDelaunayRemesher.md] and
[IndicatorThresholdCriterion.md], so one field can size the splitting, the coarsening and the
criterion together.

[!param](/Remeshing/Remeshers/TriSplitRemesher/max_splits_per_event) caps how much one event does,
taking the oversized elements with the lowest ids first. The elements the closure has to split
alongside them are not counted against the cap, because dropping those would leave hanging nodes.

The mesh must be two-dimensional and built entirely out of `TRI3` elements.

### Determinism id=determinism

The selected elements are sorted by id before anything is created, and the new nodes and elements
are then handed their ids in that order, the four-way splits first and the bisections after, each in
increasing id order. The ids an event assigns therefore depend on the mesh alone rather than on the
order the mesh iterator produced them in, so a repeated run reaches the same mesh with the same
element count. [PatchDelaunayRemesher.md] offers no such guarantee, since the mesh it produces comes
out of a triangulator.

### Solution Transfer id=transfer

Every child lies inside its parent, which makes the parent the exact source of every value the child
takes, so the record names it as the host of each new node and of each new element, and the
transfer reproduces a piecewise linear field on the children exactly. Nothing has to be located in
the old mesh, unlike a remesher whose new elements straddle several old ones. Every child inherits
the subdomain of its parent.

### Parallel Execution id=parallel

On a replicated mesh every rank holds the whole mesh and performs the identical splitting without
communication.

On a distributed mesh a rank may split only an element it owns whose neighbors it also owns, so that
the closure never has to reach onto another rank. An element at a partition seam is deferred and the
pattern is rebuilt from the reduced set, which can retire a closure split that was only there to
serve a deferred element; the console reports how many were deferred. A deferred element is not
lost, since it is still oversized at the next event. New entities are numbered out of a block of ids
each rank carves above the global maximum.

## Example Input File Syntax

!listing test/tests/remeshing/refine_front.i block=Remeshing

The size field is an ordinary auxiliary variable, so anything may compute it. Here a [ParsedAux.md]
drives it off the same [GradientJumpIndicator.md] field the criterion measures, clamped into a
positive range:

!listing test/tests/remeshing/refine_front.i block=AuxKernels/sizing

Paired with a [PatchDelaunayRemesher.md] that coarsens behind the front, the two remeshers run in
turn on one event and share the field:

!listing test/tests/remeshing/refine_coarsen_cycle.i block=Remeshing/Remeshers

!syntax parameters /Remeshing/Remeshers/TriSplitRemesher

!syntax inputs /Remeshing/Remeshers/TriSplitRemesher

!syntax children /Remeshing/Remeshers/TriSplitRemesher
