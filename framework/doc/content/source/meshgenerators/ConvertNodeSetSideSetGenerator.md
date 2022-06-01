#ConvertNodeSetSideSetGenerator

!syntax description /Mesh/ConvertNodeSetSideSetGenerator

## Overview



This MeshGenerator object allows the user to make a matching set of sides for every node set,
and a matching set of nodes for every side set. It does not delete or erase node sets or side sets.

Immediately after mesh generation,
this operation is done [automatically](MooseMesh.md#more_detail) if `construct_sideset_from_nodeset = true`.

After mesh generation, if later operations create new node sets, side sets can be constructed out of those node sets.
There needs to be at least four nodes in a node set to construct one side element (one side side with one element in it).

Currently this operation indiscriminately converts all node sets into side sets, and all side sets into node sets.
There is no way to choose which sets to perform the operation on.



!syntax parameters /Mesh/ConvertNodeSetSideSetGenerator

!syntax inputs /Mesh/ConvertNodeSetSideSetGenerator

!syntax children /Mesh/ConvertNodeSetSideSetGenerator
