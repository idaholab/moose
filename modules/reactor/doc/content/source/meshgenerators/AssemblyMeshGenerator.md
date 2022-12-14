# AssemblyMeshGenerator

!syntax description /Mesh/AssemblyMeshGenerator

## Overview

This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

The `AssemblyMeshGenerator` object generates assembly reactor geometry structures in either square or hexagonal geometries using component pin cell meshes from the [`PinMeshGenerator`](PinMeshGenerator.md) in [!param](/Mesh/AssemblyMeshGenerator/inputs). The component pin cell meshes are tagged with pin cell `reporting ID` values according to their location in the assembly grid. Any newly created regions such as ducts are given block ID assignments.

This object automates the use and functionality of the [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) for cartesian  reactor geometry, [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimensions, the [`AdvancedExtruderGenerator'](AdvancedExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment for background and duct regions (only available for hexagonal assemblies) and boundary ID and name assignment.

The `AssemblyMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but uses parameters that are more typical for reactor design. Since `AssemblyMeshGenerator` requires an input lattice structure to be defined, users that require homogenized assembly definitions or assemblies with single pins should define this structure with [`PinMeshGenerator'](PinMeshGenerator.md) and set [!param](/Mesh/PinMeshGenerator/use_as_assembly) to true. The resulting mesh will be tagged with the extra element IDs, block names, and outer boundaries in a similar manner to `AssemblyMeshGenerator`, and can be inputted directly to [`CoreMeshGenerator'](CoreMeshGenerator.md)`.

## Region ID, Block ID, and Block Name Information

The [!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) parameters are used to identify regions within the assembly around the lattice of fuel pins. This functionality is intended for easy identification of regions within the mesh that will have the same properties, such as material assignments, and this region ID will be assigned as an extra element integer.

The user defined ID assignment using [!param](/Mesh/AssemblyMeshGenerator/background_region_id) is given as a 1-D vector of size `A`, where `A` is the number of axial levels. This vector defines the background block IDs (single value per axial layer) starting from the bottom axial layer and ending with the top axial layer. Similarly, [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) is given as an `A` by `D` vector, where `D` is the number of duct intervals per axial layer. This vector assignment starts from the innermost duct region of the bottom axial layer, and extends out first radially and then axially.

For ease of use, block ids are generated automatically by the mesh generator, and for users who require element identification by block name, the optional parameters [!param](/Mesh/AssemblyMeshGenerator/background_block_name) and [!param](/Mesh/AssemblyMeshGenerator/duct_block_names) can be defined to set block names for the assembly background and duct regions respectively, where each block name will be prepended with the prefix `RGMB_ASSEMBLY<assembly_type_id>_`, where `<assembly_type_id>` is the assembly ID provided by the user through [!param](/Mesh/AssemblyMeshGenerator/assembly_type). If block names are not provided, block names will be assigned automatically to have the name `RGMB_ASSEMBLY<assembly_type_id>`.

## Reporting ID Information

As mentioned above, the `AssemblyMeshGenerator` object will tag all elements (that do not belong to one of the constituent pins) with the extra integer reporting ID named "region_id" with the value equal to the assembly region ID.

The `AssemblyMeshGenerator` object also automatically tags all elements in the mesh with the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) using the extra_integer name "assembly_type_id" and, if extruded, elements in each axial layer are tagged the axial layers using the name "plane_id". The pins composing the assembly are also tagged via [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) or [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md), using the "cell" assignment type, with the extra integer name "pin_id".

## Exterior Boundary ID Information

The `AssemblyMeshGenerator` objects automatically assigns boundary information derived from the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) parameter. The exterior assembly boundary is assigned the ID equal to 2000 + the assembly type ID and is named "outer_assembly_<assembly_type_id>" (for example an assembly with an assembly type ID of 1 will have a boundary ID of 2001 and boundary name of "outer_assembly_1").

If the assembly is extruded to three dimensions the top-most boundary ID must be assigned using [!param](/Mesh/ReactorMeshParams/top_boundary_id) and will have the name "top", while the bottom-most boundary must be assigned using [!param](/Mesh/ReactorMeshParams/bottom_boundary_id) and will have the name "bottom".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/assembly_mesh_generator/assembly_only.i block=Mesh

This is the resulting mesh block layout, where by default a single block is assigned to the triangular elements and another block is assigned to the quadrilateral elements:

!media reactor/meshgenerators/assembly_mesh_generator.png style=width:40%;

This is the resulting "region_id" extra element integer layout, which was chosen by setting the region IDs for each of the constituent pins:

!media reactor/meshgenerators/assembly_mesh_generator_rid.png style=width:40%;

!syntax parameters /Mesh/AssemblyMeshGenerator

!syntax inputs /Mesh/AssemblyMeshGenerator

!syntax children /Mesh/AssemblyMeshGenerator
