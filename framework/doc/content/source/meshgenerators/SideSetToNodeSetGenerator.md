# SideSetToNodeSetGenerator

!syntax description /Mesh/SideSetToNodeSetGenerator

## Overview


This MeshGenerator object allows the user to make a matching set of sides for every node set. It does not delete or erase node sets.

Immediately after mesh generation,
this operation is done [automatically](MooseMesh.md#more_detail) if `construct_sideset_from_nodeset = true`.

Before mesh generation, if operations create new node sets, side sets can be constructed out of those node sets.
There needs to be at least four nodes in a node set to construct one side element (one side side with one element in it).

Currently this operation indiscriminately converts all node sets into side sets, and all side sets into node sets.
There is no way to choose which sets to perform the operation on.

!listing /test/tests/meshgenerators/sideset_to_nodeset_generator/from_sides.i

!syntax parameters /Mesh/SideSetToNodeSetGenerator

!syntax inputs /Mesh/SideSetToNodeSetGenerator

!syntax children /Mesh/SideSetToNodeSetGenerator
