# RefineBlockGenerator

!syntax description /Mesh/RefineBlockGenerator

## Overview

This MeshGenerator object allows the user to refine one or more blocks in a mesh.

The user has to provide the ids of the blocks to be modified, as well as the corresponding levels of refinement for each block. These must match up to the order of the aforementioned block id list, e.g. if the blocks you wish to modify are '0 2 1 4', with 1 refinement for block 0, 2 for block 2, 3 for block 1, and 4 for block 4, then the refinement list will need to look like '1 2 3 4'. By default, refinement in libmesh refines neighboring blocks to avoid meshing problems. This generator shares this default, but it can be disabled with "enable_neighbor_refinement=false".

!syntax parameters /Mesh/RefineBlockGenerator

!syntax inputs /Mesh/RefineBlockGenerator

!syntax children /Mesh/RefineBlockGenerator
