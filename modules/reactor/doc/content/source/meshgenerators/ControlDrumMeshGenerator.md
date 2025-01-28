# ControlDrumMeshGenerator

!syntax description /Mesh/ControlDrumMeshGenerator

## Overview

This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [ReactorMeshParams](ReactorMeshParams.md), [PinMeshGenerator](PinMeshGenerator.md), [AssemblyMeshGenerator](AssemblyMeshGenerator.md), and [CoreMeshGenerator](CoreMeshGenerator.md).

This object creates the target mesh by automating the use and functionality of the [AdvancedConcentricCircleGenerator](AdvancedConcentricCircleGenerator.md) and [FlexiblePatternGenerator](FlexiblePatternGenerator.md) mesh generators to build the 2-D geometry of the control drum region and, if extruding to three dimensions, the [AdvancedExtruderGenerator](AdvancedExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [PlaneIDMeshGenerator](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID, boundary ID, and boundary name assignment for the output control drum mesh.

The `ControlDrumMeshGenerator` object generates control drum reactor geometry structures in either square or hexagonal geometries. These drum structures are created by defining two concentric rings, which creates an annular area that will be referred to as the 'drum region'. The area within the inner radius will be referred to as the 'inner drum' region, while the area outside of the outer radius will be regarded as the 'background' region. Optionally, the drum region can be sub-divided into two azimuthal sectors to distinguish the placement of the 'drum pad' region, which is commonly used in microreactor-type designs as a control mechanism to regulate the neutron absorption rate within the reactor core. A figure showing the relevant regions defined in `ControlDrumMeshGenerator` is shown in [cd_regions], where the left diagram represents the case with a drum pad explicitly defined, while the right diagram represents the case without a drum pad (i.e., the control drum is defined as a single region).

!media reactor/meshgenerators/cd_regions.png id=cd_regions style=width:70%; caption=Control drum mesh with explicit drum pad region (left) and no drum pad region (right).

The inner and outer radius of the drum region are defined by setting [!param](/Mesh/ControlDrumMeshGenerator/drum_inner_radius) and [!param](/Mesh/ControlDrumMeshGenerator/drum_outer_radius), respectively. [!param](/Mesh/ControlDrumMeshGenerator/drum_inner_intervals) controls the number of radial mesh sub-intervals in the inner drum region, while [!param](/Mesh/ControlDrumMeshGenerator/drum_intervals) sets the number of radial mesh sub-intervals in the drum region. [!param](/Mesh/ControlDrumMeshGenerator/num_azimuthal_sectors) is used to define the number of azimuthal sectors to subdivide the drum region into, while [!param](/Mesh/ControlDrumMeshGenerator/pad_start_angle) and [!param](/Mesh/ControlDrumMeshGenerator/pad_end_angle) are used to set the start and end angles of the drum pad region, respectively. Here, angles start in the positive y direction and rotate clockwise, and the difference between the end and start angles cannot exceed 360 degrees. Additionally, the start angle needs to be defined between 0 and 360 degrees, and the end angle cannot exceed 720 degrees. Additionally, if the pad angles do not line up with the azimuthal discretization of the drum region, additional azimuthal nodes are defined to ensure that the pad region as defined by the pad start and end angles lines up exactly with the azimuthal discretization of the drum region.

In order to facilitate stitching of `ControlDrumMeshGenerator` objects in downstream core lattice patterning with [CoreMeshGenerator](CoreMeshGenerator.md), users must set [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) to true to ensure that a fixed number of sectors are created at the boundary surfaces of the control drum mesh. The resulting mesh will be tagged with the extra element IDs, block names, and outer boundaries in a similar manner to `AssemblyMeshGenerator`, and can be inputted directly to [CoreMeshGenerator](CoreMeshGenerator.md).

## Region ID, Block ID, and Block Name Information

The parameter [!param](/Mesh/ControlDrumMeshGenerator/region_ids) is used to identify regions within the control drum, and this functionality is intended for easy identification of regions within the mesh that will have the same properties, such as material assignments, and this region ID will be assigned as an extra element integer.

[!param](/Mesh/ControlDrumMeshGenerator/region_ids) is given as an `A` (inner) by `R` (outer) vector, where `A` is the number of axial layer (equal to 1 for 2-D meshes) and `R` is the number of radial intervals per axial layer. Here, `R` is equal to 4 for control drum meshes with explicit drum pads defined, where the radial region ID ordering follows (drum inner, drum pad, drum ex-pad, background) according to the left image in [cd_regions]. Similarly, when the drum pad angles are not defined, `R` equals 3 and follows the ordering (drum inner, drum, background), based on the right image in [cd_regions].

For ease of use, block ids are generated automatically by the mesh generator, and for users who require element identification by block name, the optional parameter [!param](/Mesh/ControlDrumMeshGenerator/block_names) can be defined to set block names for the control drum regions. In this case, the ordering and size of the block names should match those of [!param](/Mesh/ControlDrumMeshGenerator/region_ids), and each block name will be prepended with the prefix `RGMB_DRUM<assembly_type_id>_`, where `<assembly_type_id>` is the assembly ID provided by the user through [!param](/Mesh/ControlDrumMeshGenerator/assembly_type). If not specified, the block names will be assigned automatically as `RGMB_DRUM<assembly_type_id>` by default. If [ReactorMeshParams](ReactorMeshParams.md)/[!param](/Mesh/ReactorMeshParams/region_id_as_block_name) is set to `true`, the resulting elements will have the block name `RGMB_DRUM<assembly_type_id>_REG<region_id>`, where `<region_id>` is the region ID of the element. Note that [!param](/Mesh/ReactorMeshParams/region_id_as_block_name) should not be used in conjunction with [!param](/Mesh/ControlDrumMeshGenerator/block_names). Regardless of whether block names are provided or not, the suffix `_TRI` is automatically added to the block name for all triangular elements in the central pin mesh elements when "quad_center_elements" is set to false.

## Reporting ID Information

As mentioned above, the `ControlDrumMeshGenerator` object will tag all elements (that do not belong to one of the constituent pins) with the extra integer reporting ID named "region_id" with the value equal to the drum region ID.

The `ControlDrumMeshGenerator` object also automatically tags all elements in the mesh with the [!param](/Mesh/ControlDrumMeshGenerator/assembly_type) using the extra_integer name "assembly_type_id" and, if extruded, elements in each axial layer are tagged the axial layers using the name "plane_id".

## Depletion ID Information

The `ControlDrumMeshGenerator` object can optionally assign a depletion ID, with the extra integer name "depletion_id", only if it is the final mesh generator.
The depletion ID generation option can be enabled by setting the  [!param](/Mesh/ControlDrumMeshGenerator/generate_depletion_id) to true.
The level of detail needed for depletion zones is specified in the input parameter [!param](/Mesh/ControlDrumMeshGenerator/depletion_id_type) and must be set to `pin_type`, which will assign a unique depletion ID for each radial and axial zone of the drum.

## Exterior Boundary ID Information

The `ControlDrumMeshGenerator` objects automatically assigns boundary information derived from the [!param](/Mesh/ControlDrumMeshGenerator/assembly_type) parameter. The exterior assembly boundary is assigned the ID equal to

!listing include/meshgenerators/ReactorGeometryMeshBuilderBase.h line=ASSEMBLY_BOUNDARY_ID_START =

+ the assembly type ID and is named "outer_assembly_<assembly_type_id>" (for example a control drum with an assembly type ID of 1 will have a boundary ID of 2001 and boundary name of "outer_assembly_1").

If the assembly is extruded to three dimensions the top-most boundary ID must be assigned using [!param](/Mesh/ReactorMeshParams/top_boundary_id) and will have the name "top", while the bottom-most boundary must be assigned using [!param](/Mesh/ReactorMeshParams/bottom_boundary_id) and will have the name "bottom".

## Metadata Information

Users may be interested in defining additional metadata to represent the reactor geometry and region IDs assigned to each geometry zone, which may be useful to users who want mesh geometry and composition information without having to inspect the generated mesh itself. The following metadata is defined on the control drum mesh:

- `assembly_type`: Value of type_id associated with control drum, equivalent to the input parameter [!param](/Mesh/ControlDrumMeshGenerator/assembly_type)
- `pitch`: Assembly pitch, equivalent to the input parameter [!param](/Mesh/ReactorMeshParams/assembly_pitch)
- `is_control_drum`: Whether or not this mesh is a control drum, equal to true for all structures created by `ControlDrumMeshGenerator`.
- `drum_radii`: Vector of length two corresponding to the inner and outer radii of the drum region, controlled by [!param](/Mesh/ControlDrumMeshGenerator/drum_inner_radius) and [!param](/Mesh/ControlDrumMeshGenerator/drum_outer_radius), respectively.
- `drum_region_ids`: 2-D vector of region ids corresponding to radial and axial zones within control drum regions of assembly mesh, equivalent to the input parameter [!param](/Mesh/ControlDrumMeshGenerator/region_ids). Inner indexing is radial zones, while outer index is axial zones.

In addition, the value of the `reactor_params_name` metadata can be used to retrieve global metadata defined by [ReactorMeshParams](ReactorMeshParams.md). Please refer to [ReactorMeshParams](ReactorMeshParams.md) to see a list of metadata defined by this mesh generator.

For applications where an output mesh does not need to be created and meshing routines can consist entirely of defining reactor-based metadata, the parameter `[Mesh]`/[!param](/Mesh/MeshGeneratorMesh/data_driven_generator) can be set to the mesh generator that would generate an output mesh from RGMB metadata.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/control_drum_mesh_generator/drum_pad.i block=Mesh

If [!param](/Mesh/ControlDrumMeshGenerator/pad_start_angle) and [!param](/Mesh/ControlDrumMeshGenerator/pad_end_angle) are not provided, the drum region is discretized with the same region ID applied to all azimuthal drum elements. In this case, only 3 values per axial level should be provided in [!param](/Mesh/ControlDrumMeshGenerator/region_ids) (drum inner, drum, background). In order to calculate the volume fraction of the pad region, [`MultiControlDrumFunction`](/MultiControlDrumFunction.md) can be used.

!syntax parameters /Mesh/ControlDrumMeshGenerator

!syntax inputs /Mesh/ControlDrumMeshGenerator

!syntax children /Mesh/ControlDrumMeshGenerator
