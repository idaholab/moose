# CoreMeshGenerator

!syntax description /Mesh/CoreMeshGenerator

## Overview

The `CoreMeshGenerator` object generates core-like reactor geometry structures in either square or hexagonal geometries with `reporting ID` and block ID assignments.
This object automates the use and functionality of the [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) for cartesian  reactor geometry, [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimension, the [`FancyExtruderGenerator'](FancyExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates boundary ID and name assignment. The boundaries are assigned the ID equal to 200 and the boundary name is "outer_core". 
In addition to the functionality of [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), this object allows for the definition of "empty" lattice locations using `MeshSubgenerators`. This is achieved through the use of creating "dummy" assembly meshes via [`CartesianMeshGenerator`](CartesianMeshGenerator.md) or [`HexagonConcentricCircleAdaptiveBoundaryMeshGenerator`](HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md) respectively. These assemblies are then removed after the core mesh creation via [`BlockDeletionGenerator`](BlockDeletionGenerator.md).
The `CoreMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but also adapts to use parameters that are more accessible for reactor design. 
This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), and [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md).

## Reporting ID Information

The `CoreMeshGenerator` object automatically tags the mesh, if three dimensional, with the axial layers using the name "plane_id". The assemblies composing the core are also tagged via [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), using the "cell" assignment type, with the name "assembly_id".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/core_mesh_generator/core.i block=Mesh/cmg

!syntax parameters /Mesh/CoreMeshGenerator

!syntax inputs /Mesh/CoreMeshGenerator

!syntax children /Mesh/CoreMeshGenerator

