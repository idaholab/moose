# AbaqusUELRelationshipManager

`AbaqusUELRelationshipManager` is the [RelationshipManager](RelationshipManager.md) used by the
[AbaqusUELMesh](AbaqusUELMesh.md). It is not exposed in the input file: the
[AbaqusUELMeshUserElement](AbaqusUELMeshUserElement.md) registers it automatically through its
`validParams`.

## Description

An [AbaqusUELMesh](AbaqusUELMesh.md) stores each user-element (UEL) node as a disconnected
`NodeElem`, with the actual element connectivity kept in a separate map. The standard MOOSE ghosting
based on face neighbors therefore sees no connectivity at all. `AbaqusUELRelationshipManager`
supplies the missing geometric, algebraic, and coupling ghosting from the UEL connectivity instead:
for every `NodeElem` owned by a processor, it ghosts all the other `NodeElem`s that belong to a
common UEL element (and their degrees of freedom).

This guarantees that the rank which assembles a UEL element (the owner of its first node, see
[AbaqusUELMeshUserElement](AbaqusUELMeshUserElement.md)) can read the full element solution and that
the sparsity pattern includes the cross-node coupling, even when an element's nodes are split across
ranks.

The relationship manager is registered as geometric, algebraic, and coupling:

!listing modules/solid_mechanics/src/userobjects/AbaqusUELMeshUserElement.C start=addRelationshipManager end=COUPLING include-end=True

## Implementation Details

The ghosting functor walks the same UEL connectivity that the
[AbaqusUELPartitioner](AbaqusUELPartitioner.md) uses to build its dual graph. For each element in
the queried range it looks up the incident UEL elements (`AbaqusUELMesh::getNodeToUELMap()`) and
ghosts every `NodeElem` of those elements (`AbaqusUELMesh::getElements()`) that is not already on the
target processor.

!listing modules/solid_mechanics/src/utils/AbaqusUELRelationshipManager.C start=AbaqusUELRelationshipManager::operator end=coupled_elements.emplace include-end=True

## Related Objects

- [AbaqusUELMesh](AbaqusUELMesh.md): the mesh that owns the UEL connectivity.
- [AbaqusUELMeshUserElement](AbaqusUELMeshUserElement.md): registers this relationship manager and
  assembles the UEL elements.
- [AbaqusUELPartitioner](AbaqusUELPartitioner.md): partitions the mesh using the same connectivity.
