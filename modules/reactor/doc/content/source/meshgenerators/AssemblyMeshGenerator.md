# AssemblyMeshGenerator

!syntax description /Mesh/AssemblyMeshGenerator

## Overview
This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

The `AssemblyMeshGenerator` object generates assembly reactor geometry structures in either square or hexagonal geometries using component pin cell meshes from the [`PinMeshGenerator`](PinMeshGenerator.md) in [!param](/Mesh/AssemblyMeshGenerator/inputs). The component pin cell meshes are tagged with pin cell `reporting ID` values according to their location in the assembly grid. Any newly created regions such as ducts are given block ID assignments.
This object automates the use and functionality of the [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) for cartesian  reactor geometry, [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimension, the [`FancyExtruderGenerator'](FancyExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment for background and duct regions (only available for hexagonal assemblies) and boundary ID and name assignment. 
The `AssemblyMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but but adopts the use of parameters that are more typical for reactor design. 

## Block ID Information
The [!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) parameters are used to identify regions within the assembly, that do not belong to one of the constituent pins, that belong to the same block both radially and axially. This functionality is intended for easy identification of regions within the mesh that will have the same properties, such as material assignments, and will assign the region ID to both the subdomain (block) ID and name.
The user defined ID assignment using [!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) should be given as a single value per axial layer for the background region and a vector, starting from the inner-most duct region, of the IDs for the appropriate axial layer starting from the bottom of the geometry. 
The automated ID assignment, via the [!param](/Mesh/ReactorMeshParams/procedural_region_ids) option, concatenates the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) and the peripheral radial mesh region number (indexed starting at 0 for the background region) with the maximum number of zeroes separating the two in order to differentiate the regions from regions in pins. In the case that the concatenatation exceeds the type limit, the lowest digits of the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) will be truncated. The same ID will be used for all axial layers in three dimensional meshes.

For example a hexagonal assembly with a type ID of 37 and two ducts would procedurally generate three region IDs:
- A background region ID of 37000
- A first duct region ID of 37001
- A second duct region ID of 37002

If the same assembly instead has a type ID of 65535 the same three regions would be assigned as:
- A background region ID of 65530
- A first duct region ID of 65531
- A second duct region ID of 65532

## Reporting ID Information
The `AssemblyMeshGenerator` object will tag all elements (that do not belong to one of the constituent pins) with the reporting ID named "region_id" with the value equal to their region ID whether explicitly declared or procedurally generated.
The `AssemblyMeshGenerator` object also automatically tags all elements in the mesh with the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) using the name "assembly_type_id" and, if extruded, elements in each axial layer are tagged the axial layers using the name "plane_id". The pins composing the assembly are also tagged via [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), using the "cell" assignment type, with the name "pin_id".

## Exterior Boundary ID Information
The `AssemblyMeshGenerator` objects automatically assigns boundary information derived from the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) parameter. The exterior assembly boundary is assigned the ID equal to 2000 + the assembly type ID and is named "outer_assembly_<assembly_type_id>" (for example an assembly with an assembly type ID of 1 will have a boundary ID of 2001 and boundary name of "outer_assembly_1").
If the assembly is extruded to three dimensions the top-most boundary will be assigned an ID equal to 201 and the name "top", while the bottom-most boundary will be assigned an ID equal to 202 and the name "bottom".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/assembly_mesh_generator/assembly_only.i block=Mesh/amg

!syntax parameters /Mesh/AssemblyMeshGenerator

!syntax inputs /Mesh/AssemblyMeshGenerator

!syntax children /Mesh/AssemblyMeshGenerator
