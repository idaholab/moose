# AssemblyMeshGenerator

!syntax description /Mesh/AssemblyMeshGenerator

## Overview

This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`PinMeshGenerator`](PinMeshGenerator.md), [`ControlDrumMeshGenerator`](ControlDrumMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

The `AssemblyMeshGenerator` object generates assembly reactor geometry structures in either square or hexagonal geometries using component pin cell meshes from the [`PinMeshGenerator`](PinMeshGenerator.md) in [!param](/Mesh/AssemblyMeshGenerator/inputs). The component pin cell meshes are tagged with pin cell `reporting ID` values according to their location in the assembly grid. Any newly created regions such as ducts are given block ID assignments.

This object automates the use and functionality of the [`PatternedCartesianMeshGenerator`](PatternedCartesianMeshGenerator.md) for cartesian  reactor geometry, [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md) for hexagonal reactor geometry and, if extruding to three dimensions, the [`AdvancedExtruderGenerator'](AdvancedExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md) and [`PlaneIDMeshGenerator'](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment for background and duct regions and boundary ID and name assignment.

The `AssemblyMeshGenerator` object adopts much of the existing input structure of patterned MeshGenerators but uses parameters that are more typical for reactor design. Since `AssemblyMeshGenerator` requires an input lattice structure to be defined, users that require homogenized assembly definitions or assemblies with single pins should define this structure with [`PinMeshGenerator'](PinMeshGenerator.md) and set [!param](/Mesh/PinMeshGenerator/use_as_assembly) to true. The resulting mesh will be tagged with the extra element IDs, block names, and outer boundaries in a similar manner to `AssemblyMeshGenerator`, and can be inputted directly to [`CoreMeshGenerator'](CoreMeshGenerator.md)`.

## Region ID, Block ID, and Block Name Information

The [!param](/Mesh/AssemblyMeshGenerator/background_region_id) and [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) parameters are used to identify regions within the assembly around the lattice of fuel pins. This functionality is intended for easy identification of regions within the mesh that will have the same properties, such as material assignments, and this region ID will be assigned as an extra element integer.

The user defined ID assignment using [!param](/Mesh/AssemblyMeshGenerator/background_region_id) is given as a 1-D vector of size `A`, where `A` is the number of axial levels. This vector defines the background block IDs (single value per axial layer) starting from the bottom axial layer and ending with the top axial layer. Similarly, [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) is given as an `A` by `D` vector, where `D` is the number of duct intervals per axial layer. This vector assignment starts from the innermost duct region of the bottom axial layer, and extends out first radially and then axially.

For ease of use, block ids are generated automatically by the mesh generator, and for users who require element identification by block name, the optional parameters [!param](/Mesh/AssemblyMeshGenerator/background_block_name) and [!param](/Mesh/AssemblyMeshGenerator/duct_block_names) can be defined to set block names for the assembly background and duct regions respectively. In that case, each block name will be prepended with the prefix `RGMB_ASSEMBLY<assembly_type_id>_`, where `<assembly_type_id>` is the assembly ID provided by the user through [!param](/Mesh/AssemblyMeshGenerator/assembly_type). If not specified, the block names will be assigned automatically as `RGMB_ASSEMBLY<assembly_type_id>` by default. If [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/region_id_as_block_name) is set to `true`, the resulting elements will have the block name `RGMB_ASSEMBLY<assembly_type_id>_REG<region_id>`, where `<region_id>` is the region ID of the element. Note that [!param](/Mesh/ReactorMeshParams/region_id_as_block_name) should not be used in conjunction with [!param](/Mesh/AssemblyMeshGenerator/background_block_name) or [!param](/Mesh/AssemblyMeshGenerator/duct_block_names).

## Reporting ID Information

As mentioned above, the `AssemblyMeshGenerator` object will tag all elements (that do not belong to one of the constituent pins) with the extra integer reporting ID named "region_id" with the value equal to the assembly region ID.

The `AssemblyMeshGenerator` object also automatically tags all elements in the mesh with the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) using the extra_integer name "assembly_type_id" and, if extruded, elements in each axial layer are tagged the axial layers using the name "plane_id". The pins composing the assembly are also tagged via [`PatternedCartesianMeshGenerator`](PatternedCartesianMeshGenerator.md) or [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md), using the "cell" assignment type, with the extra integer name "pin_id".

## Depletion ID Information

The `AssemblyMeshGenerator` object can optionally assign a depletion ID, with the extra integer name "depletion_id", only if they are the final mesh generator.
The depletion ID generation option can be enabled by setting the  [!param](/Mesh/AssemblyMeshGenerator/generate_depletion_id) to true.
The level of detail needed for depletion zones is specified in the input parameter [!param](/Mesh/AssemblyMeshGenerator/depletion_id_type).
[!param](/Mesh/AssemblyMeshGenerator/depletion_id_type) can be either `pin` and `pin_type`.
All pins in the assembly have separate depletion ID values by setting [!param](/Mesh/AssemblyMeshGenerator/depletion_id_type) to `pin`.
By setting that option to `pin_type`, unique ID values are assigned to individual pin types in assemblies.

## Exterior Boundary ID Information

The `AssemblyMeshGenerator` objects automatically assigns boundary information derived from the [!param](/Mesh/AssemblyMeshGenerator/assembly_type) parameter. The exterior assembly boundary is assigned the ID equal to 2000 + the assembly type ID and is named "outer_assembly_<assembly_type_id>" (for example an assembly with an assembly type ID of 1 will have a boundary ID of 2001 and boundary name of "outer_assembly_1").

If the assembly is extruded to three dimensions the top-most boundary ID must be assigned using [!param](/Mesh/ReactorMeshParams/top_boundary_id) and will have the name "top", while the bottom-most boundary must be assigned using [!param](/Mesh/ReactorMeshParams/bottom_boundary_id) and will have the name "bottom".

## Metadata Information

Users may be interested in defining additional metadata to represent the reactor geometry and region IDs assigned to each geometry zone, which may be useful to users who want mesh geometry and composition information without having to inspect the generated mesh itself. The following metadata is defined on the assembly mesh:

- `assembly_type`: Value of type_id associated with assembly, equivalent to the input parameter [!param](/Mesh/AssemblyMeshGenerator/assembly_type)
- `pitch`: Assembly pitch, equivalent to the input parameter [!param](/Mesh/ReactorMeshParams/assembly_pitch)
- `is_single_pin`: Whether or not assembly mesh is represented by a single pin region or a lattice of pins, equivalent to the input parameter [!param](/Mesh/PinMeshGenerator/use_as_assembly).
- `duct_halfpitches`: Length of apothems defining the duct locations, equivalent to the input parameter [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md)/[!param](/Mesh/AssemblyMeshGenerator/duct_halfpitch)
- `background_region_id`: 1-D vector of region_ids corresponding to axial zones of background regions of assembly mesh, equivalent to the input parameter [!param](/Mesh/AssemblyMeshGenerator/background_region_id).
- `duct_region_ids`: 2-D vector of region ids corresponding to radial and axial zones within duct regions of assembly mesh, equivalent to the input parameter [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids). Inner indexing is radial zones, while outer index is axial zones.

If the assembly is represented as a single pin, the following metadata is also defined:

- `ring_radii`: Length of ring radii comprising of assembly region, equivalent to [`PinMeshGenerator`](PinMeshGenerator.md)/[!param](/Mesh/PinMeshGenerator/ring_radii).
- `ring_region_ids`: 2-D vector of region ids corresponding to radial and axial zones within ring regions of assembly mesh, corresponding to the ring-related region ids of the input parameter [`PinMeshGenerator`](PinMeshGenerator.md)/[!param](/Mesh/PinMeshGenerator/region_ids). Inner indexing is radial zones, while outer index is axial zones.
- `is_homogenized`: Whether or not assembly mesh is homogenized, equivalent to the input parameter [`PinMeshGenerator`](PinMeshGenerator.md)/[!param](/Mesh/PinMeshGenerator/homogenized)

If instead the assembly is represented as a lattice of pins, the following metadata is defined:

- `pin_names`: List of mesh generator names of constituent pins in lattice.
- `pin_lattice`: 2-D lattice of pins in assembly, where each location represents the 0-based index of the pin in the list of names under the `pin_names` metadata entry.

For each of the pins listed in `pin_names`, the pin-level metadata is also displayed when [!param](/Mesh/AssemblyMeshGenerator/show_rgmb_metadata) is set to true. A list of pin-level metadata that is defined on the assembly mesh can be found in [PinMeshGenerator](PinMeshGenerator.md).

In addition, the value of the `reactor_params_name` metadata can be used to retrieve global metadata defined by [ReactorMeshParams](ReactorMeshParams.md). Please refer to [ReactorMeshParams](ReactorMeshParams.md) to see a list of metadata defined by this mesh generator.

For applications where an output mesh does not need to be created and meshing routines can consist entirely of defining reactor-based metadata, the parameter `[Mesh]`/[!param](/Mesh/MeshGeneratorMesh/data_driven_generator) can be set to the mesh generator that would generate an output mesh from RGMB metadata.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/assembly_mesh_generator/assembly_square.i block=Mesh

This is the resulting mesh block layout, where by default a single block is assigned to the triangular elements and another block is assigned to the quadrilateral elements:

!media reactor/meshgenerators/assembly_mesh_generator.png style=width:40%;

This is the resulting "region_id" extra element integer layout, which was chosen by setting the region IDs for each of the constituent pins:

!media reactor/meshgenerators/assembly_mesh_generator_rid.png style=width:40%;

!syntax parameters /Mesh/AssemblyMeshGenerator

!syntax inputs /Mesh/AssemblyMeshGenerator

!syntax children /Mesh/AssemblyMeshGenerator
