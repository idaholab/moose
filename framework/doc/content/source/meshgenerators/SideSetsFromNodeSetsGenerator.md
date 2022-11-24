# SideSetsFromNodeSetsGenerator

!syntax description /Mesh/SideSetsFromNodeSetsGenerator

## Overview

This MeshGenerator object allows the user to make matching side sets for every existing node set.
It does not delete or erase the original node sets in the process. A side may only be created from
several nodes if the nodes are forming a side of an element.

For example, for a 2D element, nodes from the node set must be on the two edges of a vertex of an element to make this vertex be part
of the created sideset. For a 3D hexagonal element, the four nodes on the corners of a face of an element
must be part of the original node set to make the quadrilateral face be part of the created side set.

In the current implementation, this operation indiscriminately converts +all+ node sets into side sets.

!alert note
This operation can also be performed [automatically](MooseMesh.md#more_detail) at the end of the mesh generation process
if `construct_side_list_from_node_list = true` in the `[Mesh]` block.

!listing /test/tests/meshgenerators/sidesets_from_nodesets_generator/from_nodes.i

!syntax parameters /Mesh/SideSetsFromNodeSetsGenerator

!syntax inputs /Mesh/SideSetsFromNodeSetsGenerator

!syntax children /Mesh/SideSetsFromNodeSetsGenerator
