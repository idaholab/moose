# ReactorMeshParams

!syntax description /Mesh/ReactorMeshParams

## Overview

The `ReactorMeshParams` object stores persistent mesh information about a reactor's geometry for use with [PinMeshGenerator](/PinMeshGenerator.md), [AssemblyMeshGenerator](/AssemblyMeshGenerator.md), and [CoreMeshGenerator](/CoreMeshGenerator.md). This is where the geometry type ([!param](/Mesh/ReactorMeshParams/geom) as 'Square' or 'Hex' for cartesian and hexagonal definitions respectively) and the number of dimensions of the mesh ([!param](/Mesh/ReactorMeshParams/dim) 2 or 3D) is declared and persistently enforced for the rest of the mesh definition. If the mesh is to be 3-dimensional, this is also where the axial information is declared ([!param](/Mesh/ReactorMeshParams/axial_regions) and [!param](/Mesh/ReactorMeshParams/axial_mesh_intervals)).
The option to procedurally generate region IDs is also declared in the `ReactorMeshParams` object. If [!param](/Mesh/ReactorMeshParams/procedural_region_ids) is 'true' all user declared region IDs ( [!param](/Mesh/PinMeshGenerator/region_ids), [!param](/Mesh/AssemblyMeshGenerator/background_region_id), and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids)) will be ignored in favor of the generated ones. The IDs that are assigned with this option are detailed in the report ID sections of [`PinMeshGenerator`](PinMeshGenerator.md) and [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md). This option should generally be used when detailed control over the block IDs or region IDs is not necessary.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/pin_mesh_generator/pin_only_hex_complex.i block=Mesh/rmp

!syntax parameters /Mesh/ReactorMeshParams

!syntax inputs /Mesh/ReactorMeshParams

!syntax children /Mesh/ReactorMeshParams
