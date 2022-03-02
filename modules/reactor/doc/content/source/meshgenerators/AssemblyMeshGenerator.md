# AssemblyMeshGenerator

!syntax description /Mesh/AssemblyMeshGenerator

## Overview
This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

The `AssemblyMeshGenerator` object generates assembly reactor geometry structures in either square or hexagonal geometries using component pin cell meshes from the [`PinMeshGenerator`](PinMeshGenerator.md) in [!param](/Mesh/AssemblyMeshGenerator/inputs). The component pin cell meshes are tagged with pin cell `reporting ID` values according to their location in the assembly grid. Any newly created regions such as ducts are given block ID assignments.

This object automates the use and functionality of the [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) for cartesian  reactor geometry, [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimensions, the [`FancyExtruderGenerator'](FancyExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment for background and duct regions (only available for hexagonal assemblies) and boundary ID and name assignment.

The `AssemblyMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but uses parameters that are more typical for reactor design.

## Block ID Information

The [!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) parameters are used to identify regions within the assembly, that do not belong to one of the constituent pins, that belong to the same block both radially and axially. This functionality is intended for easy identification of regions within the mesh that will have the same properties, such as material assignments, and will assign the region ID to both the subdomain (block) ID and name.

The user defined ID assignment using [!param](/Mesh/AssemblyMeshGenerator/background_region_id) is given as a 1-D vector of size `A`, where `A` is the number of axial levels. This vector defines the background block IDs (single value per axial layer) starting from the bottom axial layer and ending with the top axial layer. Similarly, [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) is given as an `A` by `D` vector, where `D` is the number of duct intervals per axial layer. This vector assignment starts from the innermost duct region of the bottom axial layer, and extends out first radially and then axially.

!alert! note title=Background and duct block IDs are modified to match background and duct region IDs
It should be noted here that the extra integer "region_id" and the block ID of the resultant background and duct elements will be modified to match the same value as specified by [!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids).
!alert-end!

## Reporting ID Information

The `AssemblyMeshGenerator` object will tag all elements (that do not belong to one of the constituent pins) with the extra integer reporting ID named "region_id" with the value equal to their region ID.

The `AssemblyMeshGenerator` object also automatically tags all elements in the mesh with the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) using the extra_integer name "assembly_type_id" and, if extruded, elements in each axial layer are tagged the axial layers using the name "plane_id". The pins composing the assembly are also tagged via [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), using the "cell" assignment type, with the extra integer name "pin_id".

## Exterior Boundary ID Information

The `AssemblyMeshGenerator` objects automatically assigns boundary information derived from the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) parameter. The exterior assembly boundary is assigned the ID equal to 2000 + the assembly type ID and is named "outer_assembly_<assembly_type_id>" (for example an assembly with an assembly type ID of 1 will have a boundary ID of 2001 and boundary name of "outer_assembly_1").

If the assembly is extruded to three dimensions the top-most boundary ID must be assigned using [!param](/Mesh/ReactorMeshParams/top_boundary_id) and will have the name "top", while the bottom-most boundary must be assigned using [!param](/Mesh/ReactorMeshParams/bottom_boundary_id) and will have the name "bottom".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/assembly_mesh_generator/assembly_only.i block=Mesh

!media reactor/meshgenerators/assembly_mesh_generator.png

!syntax parameters /Mesh/AssemblyMeshGenerator

!syntax inputs /Mesh/AssemblyMeshGenerator

!syntax children /Mesh/AssemblyMeshGenerator
