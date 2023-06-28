# ReactorMeshParams

!syntax description /Mesh/ReactorMeshParams

## Overview

The `ReactorMeshParams` object stores persistent mesh information about a reactor's geometry for use with [PinMeshGenerator](/PinMeshGenerator.md), [AssemblyMeshGenerator](/AssemblyMeshGenerator.md), and [CoreMeshGenerator](/CoreMeshGenerator.md). This is where the geometry type ([!param](/Mesh/ReactorMeshParams/geom) as 'Square' or 'Hex' for cartesian and hexagonal definitions respectively) and the number of dimensions of the mesh ([!param](/Mesh/ReactorMeshParams/dim) 2 or 3D) is declared and persistently enforced for the rest of the mesh definition. If the mesh is to be 3-dimensional, this is also where the axial information is declared ([!param](/Mesh/ReactorMeshParams/axial_regions) and [!param](/Mesh/ReactorMeshParams/axial_mesh_intervals)).

For users that require reactor-related metadata be defined onto the subsequent RGMB-based mesh generators that are called in their mesh input, the parameter [!param](/Mesh/ReactorMeshParams/generate_rgmb_metadata) can be set to 'true'. A list of metadata that is generated at the pin, assembly, and core levels can be found at [PinMeshGenerator](/PinMeshGenerator.md), [AssemblyMeshGenerator](/AssemblyMeshGenerator.md), and [CoreMeshGenerator](/CoreMeshGenerator.md), respectively.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/pin_mesh_generator/pin_only.i block=Mesh/rmp

!syntax parameters /Mesh/ReactorMeshParams

!syntax inputs /Mesh/ReactorMeshParams

!syntax children /Mesh/ReactorMeshParams
