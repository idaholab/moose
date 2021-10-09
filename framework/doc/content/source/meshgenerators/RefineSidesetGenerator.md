# RefineSidesetGenerator

!syntax description /Mesh/RefineSidesetGenerator

## Overview

This MeshGenerator object allows the user to refine one or more boundaries in a mesh, as well as neighboring elements of the boundary/boundaries.

The user has to provide the name(s) of the boundary/boundaries to be modified, as well as the corresponding levels of refinement for each boundary. These must match up to the order of the aforementioned boundary name list, e.g. if the boundaries you wish to modify are 'left right', with 1 refinement for left, 2 for right, then the refinement list will need to look like '1 2'. By default, refinement in libmesh refines neighboring boundaries to avoid meshing problems. This generator shares this default, but it can be disabled with setting [!param](/Mesh/RefineSidesetGenerator/enable_neighbor_refinement) to `false`. Additionally, the user must provide the type of refinement to perform in [!param](/Mesh/RefineSidesetGenerator/boundary_side) where "primary" merely refines the elements on the boundary, "secondary" only refines the neighbors of the boundary, and "both" refines both the elements on the boundary and its neighboring elements.

!syntax parameters /Mesh/RefineSidesetGenerator

!syntax inputs /Mesh/RefineSidesetGenerator

!syntax children /Mesh/RefineSidesetGenerator
