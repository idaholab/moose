# ReactorMeshParams

!syntax description /Mesh/ReactorMeshParams

## Overview

The `ReactorMeshParams` object stores persistent mesh information about a reactor's geometry for use with [PinMeshGenerator](/PinMeshGenerator.md), [AssemblyMeshGenerator](/AssemblyMeshGenerator.md), and [CoreMeshGenerator](/CoreMeshGenerator.md). This is where the geometry type ([!param](/Mesh/ReactorMeshParams/geom) as 'Square' or 'Hex' for cartesian and hexagonal definitions respectively) and the number of dimensions of the mesh ([!param](/Mesh/ReactorMeshParams/dim) 2 or 3D) is declared and persistently enforced for the rest of the mesh definition. If the mesh is to be 3-dimensional, this is also where the axial information is declared ([!param](/Mesh/ReactorMeshParams/axial_regions) and [!param](/Mesh/ReactorMeshParams/axial_mesh_intervals)).

## Metadata Information

The `ReactorMeshParams` object stores certain global mesh information as metadata, which can be queried from subsequent RGMB-based mesh generators. A list of metadata that is generated at the pin, assembly, and core levels can be found at [PinMeshGenerator](/PinMeshGenerator.md), [AssemblyMeshGenerator](/AssemblyMeshGenerator.md), and [CoreMeshGenerator](/CoreMeshGenerator.md), respectively, while the following metadata can be queried by passing in the name of the `ReactorMeshParams` mesh generator, which is stored at each RGMB mesh generation level with the metadata name `reactor_params_name`:

- `mesh_dimensions`: Number of dimensions in pin mesh, equivalent to  [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/dim)
- `mesh_geometry`: Whether pin geometry is hexagonal ("Hex") or Cartesian ("Square"), equivalent to  [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/geom)
- `axial_mesh_sizes`: Length of each axial region, equivalent to  [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/axial_regions). Only relevant for 3-D meshes.
- `axial_mesh_intervals`: Number of elements in the axial dimension for each axial region, equivalent to [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/axial_mesh_intervals). Only relevant for 3-D meshes.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/pin_mesh_generator/pin_square.i block=Mesh/rmp

!syntax parameters /Mesh/ReactorMeshParams

!syntax inputs /Mesh/ReactorMeshParams

!syntax children /Mesh/ReactorMeshParams
