# CoarsenBlockGenerator

!syntax description /Mesh/CoarsenBlockGenerator

## Overview

This MeshGenerator object allows the user to coarsen one or more blocks in a mesh. It has been designed
to work with previously refined meshes that were saved to a file without the mesh refinement pattern information. For example, refined meshes stored using [Exodus.md] or [VTKOutput.md] file formats do not contain the information needed to recognize parent and children elements from uniform refinement. The heuristic can still work in the general case of non-uniform refinement. The heuristic is unlikely to work on a non-previously-refined mesh that we seek to coarsen.

!alert note
If elements are refined, using a [RefineBlockGenerator.md] for example, during the mesh generation process, this generator will not be able to coarsen
them.

The user has to provide the ids of the blocks to be modified, as well as the corresponding levels of coarsening for each block. These must match up to the order of the aforementioned block id list, e.g. if the blocks you wish to modify are '0 2 1 4', with 1 coarsening for block 0, 2 for block 2, 3 for block 1, and 4 for block 4, then the coarsening list will need to look like '1 2 3 4'.

The user provides the starting point for the coarsening algorithm with the [!param](/Mesh/CoarsenBlockGenerator/starting_point) parameter. The element containing that point is found, then the algorithm attempts to coarsen by selecting each node of that element as the interior node of a coarse element containing that element. Once this element is coarsened, the neighbors of the coarse element
are considered as targets for the next round of coarsening, until all elements are coarsened once for that round of coarsening. If
the user has specified more than one round of coarsening, the algorithm is iterated.

!alert note
The starting point should be in the most refined region of the mesh
and it should also be in the subdomain that the user has selected to be the most coarsened. These are
due to limitations in the algorithm used for coarsening.

!alert note
Every block to be coarsened should be in a contiguous region of finer elements. The algorithm used for
selecting new candidates for coarsening is not able to skip an already coarse region to reach other fine
regions to coarsen.

!alert note
This mesh generator currently only works with QUAD and HEX elements and only outputs QUAD4 / HEX8 elements respectively. Please
contact a MOOSE developer if you require a different element type. If you are wanting to coarsen a triangle
or tetrahedral elements mesh though, it would probably be easier for you to just re-generate the mesh with a
coarser size.

!alert note
If the mesh has been refined in a prior simulation using MOOSE h-refinement, please use a [Checkpoint.md] `.cpr`
format, and then regular [Adaptivity](syntax/Adaptivity/index.md) to coarsen instead of this mesh generator.

!alert note
If the input mesh has non-conformalities due to prior use of adaptive mesh refinement,
this mesh generator may be able to remove them.
However if the input mesh is disjoint (for example, nodes are not stitched together between two neighbor elements), this will not be improved. You may try a [MeshRepairGenerator.md]
to stitch overlapping nodes before using a `CoarsenBlockGenerator`.

!alert warning
Sidesets containing sides that were coarsened into a coarser element side will not contain the coarse
side. This is not implemented.

!syntax parameters /Mesh/CoarsenBlockGenerator

!syntax inputs /Mesh/CoarsenBlockGenerator

!syntax children /Mesh/CoarsenBlockGenerator
