# CoreMeshGenerator

!syntax description /Mesh/CoreMeshGenerator

## Overview
This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), and [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md).

The `CoreMeshGenerator` object generates core-like reactor geometry structures in either square or hexagonal geometries with `reporting ID` and block ID assignments. There is expected to be only a single `CoreMeshGenerator` in a Mesh definition.
This object automates the use and functionality of the [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) for cartesian  reactor geometry, [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimension, the [`FancyExtruderGenerator'](FancyExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates boundary ID and name assignment. 
In addition to the functionality of [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), this object allows for the definition of "empty" lattice locations using `MeshSubgenerators`. This is achieved through the use of creating "dummy" assembly meshes via [`CartesianMeshGenerator`](CartesianMeshGenerator.md) or [`HexagonConcentricCircleAdaptiveBoundaryMeshGenerator`](HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md) respectively. These assemblies are then removed after the core mesh creation via [`BlockDeletionGenerator`](BlockDeletionGenerator.md).
The `CoreMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but also adapts to use parameters that are more accessible for reactor design. 

## Reporting ID Information

The `CoreMeshGenerator` object automatically tags the mesh, if three dimensional, with the axial layers using the name "plane_id". The assemblies composing the core are also tagged via [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), using the "cell" assignment type, with the name "assembly_id" and any "dummy" assembly (identified via the [!param](/Mesh/CoreMeshGenerator/dummy_assembly_name) parameter) locations excluded.

## Exterior Boundary ID Information
The `CoreMeshGenerator` objects automatically assigns boundary information. The exterior core boundary is assigned the ID equal to 200 is named "outer_core".
If the core is extruded to three dimensions the top-most boundary will be assigned an ID equal to 201 and the name "top", while the bottom-most boundary will be assigned an ID equal to 202 and the name "bottom".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/core_mesh_generator/core.i block=Mesh/cmg

!syntax parameters /Mesh/CoreMeshGenerator

!syntax inputs /Mesh/CoreMeshGenerator

!syntax children /Mesh/CoreMeshGenerator
