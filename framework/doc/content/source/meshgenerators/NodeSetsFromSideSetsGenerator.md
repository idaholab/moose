# NodeSetsFromSideSetsGenerator

!syntax description /Mesh/NodeSetsFromSideSetsGenerator

## Overview

This MeshGenerator object allows the user to generate a corresponding node set for every side set in the mesh.
It does not delete or erase side sets.

In the current implementation, this operation indiscriminately converts +all+ side sets into node sets.

!alert note
This operation can also be performed [automatically](MooseMesh.md#more_detail) at the end of the mesh generation
process if `construct_node_list_from_side_list = true` in the `[Mesh]` block.

!listing /test/tests/meshgenerators/nodesets_from_sidesets_generator/from_sides.i

!syntax parameters /Mesh/NodeSetsFromSideSetsGenerator

!syntax inputs /Mesh/NodeSetsFromSideSetsGenerator

!syntax children /Mesh/NodeSetsFromSideSetsGenerator
