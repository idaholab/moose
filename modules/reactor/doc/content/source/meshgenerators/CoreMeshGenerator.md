# CoreMeshGenerator

!syntax description /Mesh/CoreMeshGenerator

## Overview

This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), and [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md).

The `CoreMeshGenerator` object generates core-like reactor geometry structures in either square or hexagonal geometries with block ID assignments and reporting (extra integer) IDs, as described in [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md and [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md). There is expected to only be a single `CoreMeshGenerator` in a Mesh definition.

This object automates the use and functionality of the [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) for cartesian  reactor geometry, [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimensions, the [`AdvancedExtruderGenerator'](AdvancedExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates boundary ID and name assignment.

In addition to the functionality of [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), this object allows for the definition of "empty" lattice locations using `MeshSubgenerators`. This is achieved through the use of creating "dummy" assembly meshes via [`CartesianMeshGenerator`](CartesianMeshGenerator.md) or [`HexagonConcentricCircleAdaptiveBoundaryMeshGenerator`](HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md) respectively. These assemblies are then removed after the core mesh creation via [`BlockDeletionGenerator`](BlockDeletionGenerator.md). If assembly homogenization is leveraged by setting both [!param](/Mesh/PinMeshGenerator/use_as_assembly) and [!param](/Mesh/PinMeshGenerator/homogenized) to true, then all assemblies inputted to `CoreMeshGenerator` must be homogenized. Mixtures of heterogeneous and homogeneous assembly inputs to `CoreMeshGenerator` are not currently supported.

The `CoreMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but also adapts to use parameters that are more accessible for reactor design.

## Reporting ID Information

The `CoreMeshGenerator` object automatically tags the mesh, if three dimensional, with the axial layers using the extra integer name "plane_id". The assemblies composing the core are also tagged via [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), using the "cell" assignment type, with the extra integer name "assembly_id" and any "dummy" assembly (identified via the [!param](/Mesh/CoreMeshGenerator/dummy_assembly_name) parameter) locations excluded.

## Exterior Boundary ID Information

The `CoreMeshGenerator` objects automatically assigns boundary information. The exterior core boundary ID is assigned with the parameter [!param](/Mesh/ReactorMeshParams/radial_boundary_id) and will have the name "outer_core".

If the core is extruded to three dimensions the top-most boundary ID must be assigned using [!param](/Mesh/ReactorMeshParams/top_boundary_id) and will have the name "top", while the bottom-most boundary must be assigned using [!param](/Mesh/ReactorMeshParams/bottom_boundary_id) and will have the name "bottom".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/core_mesh_generator/core.i block=Mesh

This is the resulting mesh block layout, where by default a single block is assigned to all of the quadrilateral elements in the mesh:

!media reactor/meshgenerators/core_mesh_generator.png style=width:40%;

This is the resulting "region_id" extra element integer layout, which was chosen by setting the region IDs for each of the constituent pins and assemblies:

!media reactor/meshgenerators/core_mesh_generator_rid.png style=width:40%;

## Periphery Mesh Generation

The `CoreMeshGenerator` includes support for meshing a circular reactor periphery surrounding the core. This integration supports using either [`PeripheralTriangleMeshGenerator`](PeripheralTriangleMeshGenerator.md) (PTMG) or [`PeripheralRingMeshGenerator`](PeripheralRingMeshGenerator.md) (PRMG), selected using the [!param](/Mesh/CoreMeshGenerator/periphery_generator) input option (by specifying either `triangle` or `quad_ring`, respectively). The input options for these mesh generators are provided below, but more details on their meaning and usage can be found in their respective documentation pages. The generated periphery region is given the block name [!param](/Mesh/CoreMeshGenerator/periphery_block_name) (default `RGMB_CORE`) and extra integer reporting ID `region_id` [!param](/Mesh/CoreMeshGenerator/periphery_region_id), along with outer boundary name "outside_periphery".

## Example Core Periphery Syntax

!listing modules/reactor/test/tests/meshgenerators/core_mesh_generator/core_periphery_ptmg.i block=Mesh

This is the resulting mesh block layout:

!media reactor/meshgenerators/core_mesh_generator_ptmg.png style=width:40%;

!syntax parameters /Mesh/CoreMeshGenerator

!syntax inputs /Mesh/CoreMeshGenerator

!syntax children /Mesh/CoreMeshGenerator
