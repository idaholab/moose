# NodeSetToSideSetGenerator

!syntax description /Mesh/NodeSetToSideSetGenerator

## Overview


This MeshGenerator object allows the user to make a matching set of nodes for every side set. It does not delete or erase side sets in the process.

Immediately after mesh generation,
this operation is done [automatically](MooseMesh.md#more_detail) if `construct_sideset_from_nodeset = true`.

Before mesh generation, if operations create new node sets, side sets can be constructed out of those node sets.
There needs to be at least four nodes in a node set to construct one side element (one side side with one element in it).

Currently this operation indiscriminately converts all node sets into side sets, and all side sets into node sets.
There is no way to choose which sets to perform the operation on.

!listing /test/tests/meshgenerators/nodeset_to_sideset_generator/from_nodes.i

!syntax parameters /Mesh/NodeSetToSideSetGenerator

!syntax inputs /Mesh/NodeSetToSideSetGenerator

!syntax children /Mesh/NodeSetToSideSetGenerator
