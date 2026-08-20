# AbaqusUELPartitioner

`AbaqusUELPartitioner` is the mesh partitioner used by the [AbaqusUELMesh](AbaqusUELMesh.md). It
distributes the user-element (UEL) mesh across MPI ranks for parallel execution. It is not a
user-facing object: the [AbaqusUELMesh](AbaqusUELMesh.md) installs it automatically, so no input
syntax is required.

## Description

An [AbaqusUELMesh](AbaqusUELMesh.md) cannot be represented as a regular libMesh mesh. Each UEL node
is stored as a disconnected `NodeElem` (a single-node "point" element), and the actual element
connectivity lives in a separate UEL connectivity map. Because the `NodeElem`s have no sides, they
have no face neighbors, and the dual graph that the standard Metis/ParMETIS partitioners build from
face neighbors is empty. With an empty graph those partitioners have nothing to cut, which is why a
plain linear partitioner (or a space-filling-curve partitioner, which is not enabled in MOOSE by
default) was previously required.

`AbaqusUELPartitioner` solves this by building the dual graph from the UEL connectivity instead of
from face neighbors: two `NodeElem`s are connected in the graph if they are both nodes of a common
UEL element. The resulting graph is partitioned with ParMETIS through PETSc's `MatPartitioning`
interface. This produces a connectivity-aware, load-balanced partitioning that keeps the nodes of
each UEL element close together, minimizing the parallel coupling that the
[AbaqusUELMeshUserElement](AbaqusUELMeshUserElement.md) has to assemble across ranks.

## Implementation Details

### Installation

The partitioner is constructed and installed by [AbaqusUELMesh](AbaqusUELMesh.md) in its
constructor through `MooseMesh::setCustomPartitioner()`, which clones the supplied object and stores
the clone. A user-supplied `[Partitioner]` block still overrides this default.

!listing modules/solid_mechanics/src/mesh/AbaqusUELMesh.C start=AbaqusUELMesh::AbaqusUELMesh end=setCustomPartitioner include-end=True

### Building the dual graph

The class derives directly from `libMesh::Partitioner` and overrides `_do_partition()`. The dual
graph is the protected `_dual_graph` member that all libMesh partitioners use: a vector of
adjacency lists indexed by local element, where the entries are the contiguous, partitioning
independent global element indices produced by `_find_global_index_by_pid_map()`.

For each locally owned `NodeElem` (a graph vertex), the partitioner walks the UEL connectivity
exposed by [AbaqusUELMesh](AbaqusUELMesh.md) (`getNodeToUELMap()` and `getElements()`): the node's
incident UEL elements are looked up, and every other node of those elements becomes a graph
neighbor. This is the same traversal performed by the
[AbaqusUELRelationshipManager](AbaqusUELRelationshipManager.md), which ghosts exactly these
`NodeElem`s (and their DOFs). Because that relationship manager is geometric, the neighbor elements
are available in the global index map when the graph is built.

!listing modules/solid_mechanics/src/mesh/AbaqusUELPartitioner.C start=AbaqusUELPartitioner::_do_partition end=assign_partitioning include-end=True

### Partitioning the graph

The graph is handed to `PetscExternalPartitioner::partitionGraph()`, a static helper that wraps an
arbitrary adjacency graph in a PETSc `MatMPIAdj` and runs it through `MatPartitioning` with the
`parmetis` package. The returned per-element partition is applied with the standard
`Partitioner::assign_partitioning()`. Reusing this helper means the implementation relies entirely
on PETSc/ParMETIS, which is always available in a MOOSE build, rather than on the optional
space-filling-curve libraries.

### Replicated meshes

ParMETIS expects a distributed graph, i.e. every rank must own a chunk of the elements. On a
replicated mesh every rank holds every element, so `partition()` first applies a linear
partitioning to hand each rank a contiguous block before the dual graph is built. This mirrors
`PetscExternalPartitioner::preLinearPartition()`.

## Related Objects

- [AbaqusUELMesh](AbaqusUELMesh.md): the mesh that installs and owns this partitioner.
- [AbaqusUELMeshUserElement](AbaqusUELMeshUserElement.md): assembles the UEL elements; processes
  each element on the rank that owns its first node.
- [AbaqusUELRelationshipManager](AbaqusUELRelationshipManager.md): ghosts the off-processor
  `NodeElem`s of each UEL element, using the same connectivity the partitioner uses to build the
  graph.
