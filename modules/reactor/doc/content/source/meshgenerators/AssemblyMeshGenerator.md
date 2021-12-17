# AssemblyMeshGenerator

!syntax description /Mesh/AssemblyMeshGenerator

## Overview

The `AssemblyMeshGenerator` object generates assembly-like reactor geometry structures in either square or hexagonal geometries with `reporting ID` and block ID assignments.
This object automates the use and functionality of the [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) for cartesian  reactor geometry, [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimension, the [`FancyExtruderGenerator'](FancyExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment for background and duct regions (only available for hexagonal assemblies) and boundary ID and name assignment. The boundaries are assigned the ID equal to 2000+[!param](/Mesh/AssemblyMeshGenerator/assembly_type) and the boundary name is the concatenatation of "outer_assembly_" and the [!param](/Mesh/AssemblyMeshGenerator/assembly_type). 
The `AssemblyMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but also adapts to use parameters that are more accessible for reactor design. 
This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

## Reporting ID Information

The `AssemblyMeshGenerator` object allows for either user defined ([!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids)) or automated ID assignments for additional material regions that were not assigned in the [`PinMeshGenerators`](PinMeshGenerator.md) that compose the input [!param](/Mesh/AssemblyMeshGenerator/pattern). These IDs are for background and duct regions and are assigned to the block IDs, block names, and element intergers with the tag "region_id".

The automated ID assignment concatenates the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) and the peripheral radial mesh region number (indexed starting at 0 for the background region) with the maximum number of zeroes separating the two in order to differentiate the regions from regions in pins. In the case that the concatenatation exceeds the type limit, the lowest digits of the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) will be truncated. The same ID will be used for all axial layers in three dimensional meshes.
The user defined ID assignment using [!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) should be given as a single value per axial layer for the background region and a vector, starting from the inner-most duct region, of the IDs for the appropriate axial layer starting from the bottom of the geometry. 

The `AssemblyMeshGenerator` object also automatically tags the mesh with the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) using the name "assembly_type_id" and, if extruded, the axial layers using the name "plane_id". The pins composing the assembly are also tagged via [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), using the "cell" assignment type, with the name "pin_id".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/assembly_mesh_generator/assembly_only.i block=Mesh/amg

!syntax parameters /Mesh/AssemblyMeshGenerator

!syntax inputs /Mesh/AssemblyMeshGenerator

!syntax children /Mesh/AssemblyMeshGenerator
