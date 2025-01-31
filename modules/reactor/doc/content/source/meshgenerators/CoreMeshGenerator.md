# CoreMeshGenerator

!syntax description /Mesh/CoreMeshGenerator

## Overview

This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md), and [`ControlDrumMeshGenerator`](ControlDrumMeshGenerator.md).

The `CoreMeshGenerator` object generates core-like reactor geometry structures in either square or hexagonal geometries with block ID assignments and reporting (extra integer) IDs, as described in [`PatternedCartesianMeshGenerator`](PatternedCartesianMeshGenerator.md) and [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md). There is expected to only be a single `CoreMeshGenerator` in a Mesh definition.

This object automates the use and functionality of the [`PatternedCartesianMeshGenerator`](PatternedCartesianMeshGenerator.md) for cartesian  reactor geometry, [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimensions, the [`AdvancedExtruderGenerator'](AdvancedExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates boundary ID and name assignment.

In addition to the functionality of [`PatternedCartesianMeshGenerator`](PatternedCartesianMeshGenerator.md) or [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md), this object allows for the definition of "empty" lattice locations using `MeshSubgenerators`. This is achieved through the use of creating "dummy" assembly meshes via [`CartesianMeshGenerator`](CartesianMeshGenerator.md) or [`HexagonConcentricCircleAdaptiveBoundaryMeshGenerator`](HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md) respectively. These assemblies are then removed after the core mesh creation via [`BlockDeletionGenerator`](BlockDeletionGenerator.md). If assembly homogenization is leveraged by setting both [!param](/Mesh/PinMeshGenerator/use_as_assembly) and [!param](/Mesh/PinMeshGenerator/homogenized) to true, then all assemblies inputted to `CoreMeshGenerator` must be homogenized. Mixtures of heterogeneous and homogeneous assembly inputs to `CoreMeshGenerator` are not currently supported.

The `CoreMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but also adapts to use parameters that are more accessible for reactor design.

## Reporting ID Information

The `CoreMeshGenerator` object automatically tags the mesh, if three dimensional, with the axial layers using the extra integer name "plane_id". The assemblies composing the core are also tagged via [`PatternedCartesianMeshGenerator`](PatternedCartesianMeshGenerator.md) or [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md), using the "cell" assignment type, with the extra integer name "assembly_id" and any "dummy" assembly (identified via the [!param](/Mesh/CoreMeshGenerator/dummy_assembly_name) parameter) locations excluded.

## Depletion ID Information

The `CoreMeshGenerator` object can optionally assign a depletion ID, with the extra integer name "depletion_id".
The depletion ID generation option can be enabled by setting the  [!param](/Mesh/CoreMeshGenerator/generate_depletion_id) to true.
The level of detail needed for depletion zones is specified in the input parameter [!param](/Mesh/CoreMeshGenerator/depletion_id_type).
For a core with heterogeneous assemblies, [!param](/Mesh/CoreMeshGenerator/depletion_id_type) can be either `pin` and `pin_type`.
All pins in the core have separate depletion ID values by setting [!param](/Mesh/CoreMeshGenerator/depletion_id_type) to `pin`.
By setting that option to `pin_type`, unique ID values are assigned to individual pin types in assemblies.
However, pins in different assemblies have different depletion IDs even if they have the same type.
For a core with homogenized assemblies (no explicit pins), [!param](/Mesh/CoreMeshGenerator/depletion_id_type) can be either `assembly` and `assembly_type`, which assign unique ID values to individual assemblies or to individual assembly types, respectively.

## Exterior Boundary ID Information

The `CoreMeshGenerator` objects automatically assigns boundary information. The exterior core boundary ID is assigned with the parameter [!param](/Mesh/ReactorMeshParams/radial_boundary_id) and will have the name "outer_core".

If the core is extruded to three dimensions the top-most boundary ID must be assigned using [!param](/Mesh/ReactorMeshParams/top_boundary_id) and will have the name "top", while the bottom-most boundary must be assigned using [!param](/Mesh/ReactorMeshParams/bottom_boundary_id) and will have the name "bottom".

## Flexible Assembly Stitching

By default, `CoreMeshGenerator` will stitch assemblies created by [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md) together without regard for the number and location of nodes at the exterior boundaries of the assemblies. This works if very similar assemblies are being stitched together. However, this will lead to an output core mesh with hanging nodes if dissimilar assemblies are being stitched together. The following situations are identified as scenarios where such hanging nodes can occur between stitched assemblies:

1. Two assemblies have the same constituent pin geometry but vary in total number of pins in the pin lattice
2. Two assemblies have the same pin lattice structure and geometry, but the constituent pins of each assembly are subdivided into a different number of sectors per side.
3. One assembly is defined as a heterogeneous mesh (contains one or more pins), and the other assembly is homogenized.

`CoreMeshGenerator` will throw a warning if it detects that assembly stitching may lead to hanging nodes. If this happens, the user can regenerate the core mesh by setting [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) to `true` to enable flexible assembly stitching. This flexible assembly stitching algorithm deletes the outermost mesh interval and replaces it with a triangulated region using [`FlexiblePatternGenerator`](FlexiblePatternGenerator.md). For a homogeneous assembly, the entire assembly region is triangulated. By doing so, the number of nodes at the outer boundary of each input assembly will be identical and positioned at the same locations, thus enabling stitching of dissimilar assemblies. In order to control the number of sectors at the outer assembly boundary after the triangulation step, the user can set this parameter using [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/num_sectors_at_flexible_boundary). If the core lattice consists of any structures created with [ControlDrumMeshGenerator](ControlDrumMeshGenerator.md), then [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) must be set to `true`. The following three images describe how flexible assembly patterning can be used to address the issue of hanging nodes for the three cases listed above:

!media reactor/meshgenerators/rgmb_flexible_stitching_case1.png style=width:70%;

!media reactor/meshgenerators/rgmb_flexible_stitching_case2.png style=width:70%;

!media reactor/meshgenerators/rgmb_flexible_stitching_case3.png style=width:70%;

## Metadata Information

Users may be interested in defining metadata to represent the reactor geometry and region IDs assigned to each geometry zone, which may be useful to users who want mesh geometry and composition information without having to inspect the generated mesh itself. At the core level, the following metadata is defined on the output mesh:

- `assembly_names`: Mesh generator names of input assemblies in lattice, similar to input parameter (/Mesh/CoreMeshGenerator/inputs) but with the dummy assembly name excluded
- `lattice`: 2-D lattice of assemblies in core, where each location represents the 0-based index of the assembly in the list of names under the `assembly_names` metadata entry. A -1 entry represents a dummy assembly.

For each of the assemblies listed in `assembly_names`, the assembly-level metadata is also displayed. In addition, if any of these assemblies are comprised of pins in a lattice, the pin-level metadata of these constituent pins is also displayed. A list of assembly-level and pin-level metadata defined on the core mesh can be found in [AssemblyMeshGenerator](AssemblyMeshGenerator.md) and [PinMeshGenerator](PinMeshGenerator.md) respectively.

For meshes where a core periphery is defined, the following metadata is also defined:

- `peripheral_ring_radius`: Outer radius of core periphery, equivalent to the input parameter [`CoreMeshGenerator`](CoreMeshGenerator.md)/[!param](/Mesh/CoreMeshGenerator/outer_circle_radius).
- `peripheral_ring_region_id`: Region ID associated with core periphery, equivalent to the input parameter [`CoreMeshGenerator`](CoreMeshGenerator.md)/[!param](/Mesh/CoreMeshGenerator/periphery_region_id).

In addition, the value of the metadata `reactor_params_name` can be used to retrieve global metadata defined by [ReactorMeshParams](ReactorMeshParams.md). Please refer to [ReactorMeshParams](ReactorMeshParams.md) to see a list of metadata defined by this mesh generator.

For applications where an output mesh does not need to be created and meshing routines can consist entirely of defining reactor-based metadata, the parameter `[Mesh]`/[!param](/Mesh/MeshGeneratorMesh/data_driven_generator) can be set to the mesh generator that would generate an output mesh from RGMB metadata.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/core_mesh_generator/core_square.i block=Mesh

This is the resulting mesh block layout, where by default a single block is assigned to all of the quadrilateral elements in the mesh:

!media reactor/meshgenerators/core_mesh_generator.png style=width:40%;

This is the resulting "region_id" extra element integer layout, which was chosen by setting the region IDs for each of the constituent pins and assemblies:

!media reactor/meshgenerators/core_mesh_generator_rid.png style=width:40%;

## Periphery Mesh Generation

The `CoreMeshGenerator` includes support for meshing a circular reactor periphery surrounding the core. This integration supports using either [`PeripheralTriangleMeshGenerator`](PeripheralTriangleMeshGenerator.md) (PTMG) or [`PeripheralRingMeshGenerator`](PeripheralRingMeshGenerator.md) (PRMG), selected using the [!param](/Mesh/CoreMeshGenerator/periphery_generator) input option by specifying either `triangle` or `quad_ring`, respectively. The input options for these mesh generators are provided below, but more details on their meaning and usage can be found in their respective documentation pages. The generated periphery region is given the block name [!param](/Mesh/CoreMeshGenerator/periphery_block_name) (default `RGMB_CORE`) and extra integer reporting ID `region_id` [!param](/Mesh/CoreMeshGenerator/periphery_region_id), along with outer boundary name "outside_periphery". If [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/region_id_as_block_name) is set to `true`, the resulting element will have the block name `RGMB_CORE_REG<region_id>`, where `<region_id>` is the region ID of the element. Note that [!param](/Mesh/ReactorMeshParams/region_id_as_block_name) should not be used in conjunction with [!param](/Mesh/CoreMeshGenerator/periphery_block_name).

## Example Core Periphery Syntax

!listing modules/reactor/test/tests/meshgenerators/core_mesh_generator/core_periphery_ptmg_vol.i block=Mesh

This is the resulting mesh block layout:

!media reactor/meshgenerators/core_mesh_generator_ptmg.png style=width:40%;

!syntax parameters /Mesh/CoreMeshGenerator

!syntax inputs /Mesh/CoreMeshGenerator

!syntax children /Mesh/CoreMeshGenerator
