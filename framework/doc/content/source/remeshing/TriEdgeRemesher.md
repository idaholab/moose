# TriEdgeRemesher

!syntax description /Remeshing/Remeshers/TriEdgeRemesher

## Description

This remesher adapts a `TRI3` mesh toward the target element size field
[!param](/Remeshing/Remeshers/TriEdgeRemesher/sizing_variable) with the three local edge operations
of a metric-based remesher such as Mmg: edge splitting, edge collapse and edge swapping. Every edge
is measured in the metric the field induces, which is its Euclidean length divided by the target
size interpolated to it. An edge longer than $\sqrt{2}$ in that measure is split at its midpoint, an
edge shorter than $1/\sqrt{2}$ is collapsed by removing one of its vertices, and an edge whose swap
strictly improves the worse shape quality of its two triangles is swapped. The $\sqrt{2}$ pair is
the classical choice that keeps the operations out of each other's output: the children of a split
land above the collapse bound, and a collapse that would create an edge above the split bound is
refused. An edge between the bounds is at rest, so a mesh that meets its target passes through an
event untouched.

Refinement and coarsening therefore come out of one operator set and one field, where
[TriSplitRemesher.md] and [PatchDelaunayRemesher.md] divide the same work between two objects with a
dead band negotiated between them. The whole mesh is measured on every event, not only the region a
criterion fired on, so a mesh that disagrees with its sizing field anywhere is driven toward it: a
background built finer than the field's ceiling coarsens on the first event. The
[!param](/Remeshing/RemeshingAction/initial_remesh_cycles) of the [Remeshing](syntax/Remeshing/index.md) block runs that
equilibration on the initial condition, before the transient starts, so that the first time step is
solved on a mesh that already meets the field.

Exterior boundaries, sidesets and subdomain seams pass through the surgery unchanged: a vertex on
any of them is never removed by a collapse, and an edge on any of them is never swapped. Such an
edge may still be split, and its midpoint inherits its boundary ids, so refinement tracks a moving
boundary the way the splitting remesher does. A split here is a plain bisection of the one or two
triangles on the edge rather than a red-green pattern: the midpoint bisects both neighbors by
construction, so no closure is needed and no hanging node is possible. The pinning of seam vertices
means a subdomain seam retains the refinement history laid down along it; a region that must
coarsen back completely should not be delimited by subdomain boundaries.

[!param](/Remeshing/Remeshers/TriEdgeRemesher/min_element_size) holds the target at a floor the way
the same parameter of [TriSplitRemesher.md] does: where the field asks for less, the floor takes
over, and the field may fall to zero or below there. Without it the field itself must be positive
on every element. [!param](/Remeshing/Remeshers/TriEdgeRemesher/max_iterations) bounds how many
split-collapse-swap rounds one event performs; a round leaves at most a factor of two in edge
length, and an event stops early once a round changes nothing.

### Determinism id=determinism

The operations are performed on a shadow copy of the mesh, built with its vertices in increasing
node id order and its triangles in increasing element id order, and the candidate edges of every
pass are processed in sorted order. Every operation is exact arithmetic on nodes and midpoints
already decided, so a repeated run reaches the same mesh with the same element count, which the
triangulator-backed [PatchDelaunayRemesher.md] cannot promise. The shadow exists because the
Remesher contract keeps every replaced element in the mesh until the engine has read the old
solution through it, and chaining local operations requires the intermediate states to be mutable;
an element whose shadow triangle survives every pass is left alone.

### Solution Transfer id=transfer

Every new node and every new triangle centroid lies inside the union of the elements the event
replaces, so its source is located among them while they are still in the mesh, following the
[PatchDelaunayRemesher.md] transfer contract. A collapse or swap genuinely resamples the field onto
triangles that are not children of the ones they replace, so the solution integral moves by the
resampling error across such an event, the way it does across a retriangulated cavity.

### Parallel Execution id=parallel

This remesher runs on replicated meshes only, where every rank holds the whole mesh and
performs the identical surgery; a distributed mesh is refused when the remesher is constructed. The
target size is only readable on the elements a rank owns, so the targets are gathered before the
shadow is built, the way the patch Delaunay remesher gathers them.

## Example Input File Syntax

!listing test/tests/remeshing/edge_remesh_cycle.i block=Remeshing

The size field is an ordinary auxiliary variable computed off the same indicator the criterion
measures, and the remesher's floor is what keeps its target positive at the front:

!listing test/tests/remeshing/edge_remesh_cycle.i block=AuxKernels/sizing

!syntax parameters /Remeshing/Remeshers/TriEdgeRemesher

!syntax inputs /Remeshing/Remeshers/TriEdgeRemesher

!syntax children /Remeshing/Remeshers/TriEdgeRemesher
