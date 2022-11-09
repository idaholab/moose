# PinMeshGenerator

!syntax description /Mesh/PinMeshGenerator

## Overview

This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

The `PinMeshGenerator` object generates square or hexagonal reactor geometry pin cell structures which may be combined into larger assembly structures using `AssemblyMeshGenerator`. The block IDs, external boundary ID, region IDs (e.g., materials), and reporting IDs (extra element integers identifying unique planes and pins, as described in [`CartesianIDPatternedMeshGenerator`](CartesianIDPatternedMeshGenerator.md) and [`HexIDPatternedMeshGenerator`](HexIDPatternedMeshGenerator.md) are automatically assigned once the user provides some basic information.

This pin may be extruded to three dimensions by setting [!param](/Mesh/PinMeshGenerator/extrude) to 'true', however such extruded pins cannot be used as input to `AssemblyMeshGenerator`. Instead, 2-D pins must be inputted to `AssemblyMeshGenerator` and [!param](/Mesh/AssemblyMeshGenerator/extrude) should be set to 'true' at the `AssemblyMeshGenerator` definition to extrude the assembly to 3-D.


The `PinMeshGenerator` object automates the use and functionality of the [`PolygonConcentricCircleMeshGenerator`](PolygonConcentricCircleMeshGenerator.md) and, if extruding to three dimensions, the [`AdvancedExtruderGenerator`](AdvancedExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`TransformGenerator`](TransformGenerator.md), [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md), and [`PlaneIDMeshGenerator`](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment and boundary ID and name assignment.

The `PinMeshGenerator` object adopts much of the existing input structure of [`PolygonConcentricCircleMeshGenerator`](PolygonConcentricCircleMeshGenerator.md) but uses parameters that are more typical for reactor design. Setting [!param](/Mesh/PinMeshGenerator/homogenized) to true generates a homogenized pin structure that calls [`SimpleHexagonGenerator`](SimpleHexagonGenerator.md). This currently only works for hexagonal geometries, and whether a triangular or quadrilateral discretization is used for homogenization depends on the value of [!param](/Mesh/PinMeshGenerator/quad_center_elements). In addition, setting the parameter [!param](/Mesh/PinMeshGenerator/use_as_assembly) to true defines the output pincell structure as a single assembly, ensuring that the block names, outer boundaries, and extra element integers match the conventions used by [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md). This option should be used if defining single assemblies that are to be inputted directly to [`CoreMeshGenerator`](CoreMeshGenerator.md), since [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md) requires the assembly to be composed of multiple pins in a lattice structure.

## Region ID, Block ID, and Block Name Information

The [!param](/Mesh/PinMeshGenerator/region_ids) parameter provides a map of "region_id" values to assign to zones in the pin mesh. Each row in this map corresponds to a single axial layer of the pin and contains individual entries corresponding to the radial zones within the pin, starting from the centermost region and extending radially outward. The number of columns (entries in the row) should be identical to the number of rings + 1 (background region) + number of ducts. The required number of rows is dependent on the number of axial layers in the pin. For 2D pins, a single row of entries should be provided. For 3D pins, multiple rows must be provided (one for each axial layer). For 3D pins, the top row corresponds to the bottom of the pin cell.

The region_ids parameter entries can conveniently be selected to match material ids to be assigned to each region of the problem. Using the same value in multiple entries of the [!param](/Mesh/PinMeshGenerator/region_ids) parameter will effectively assign elements in multiple zones to the same region_id.

Region IDs are mapped to the mesh as an extra element integer, where the integer value for each mesh element will match the information provided in [!param](/Mesh/PinMeshGenerator/region_ids). For ease of use, block ids are generated automatically by the mesh generator, and for users who require element identification by block name, the optional parameter [!param](/Mesh/PinMeshGenerator/block_names) can be defined to set block names in the same manner as [!param](/Mesh/PinMeshGenerator/region_ids). In the resulting mesh, each block name will be prepended with the prefix `RGMB_PIN<pin_type_id>_`, where `<pin_type_id>` is the pin ID provided by the user through [!param](/Mesh/PinMeshGenerator/pin_type). If block names are not provided by the user, block names will be assigned automatically to have the name `RGMB_PIN<pin_type_id>`. Regardless of whether block names are provided are not, the suffix `_TRI` is automatically added to the block name for all triangular elements in the central pin mesh elements when [!param](/Mesh/PinMeshGenerator/quad_center_elements) is set to false. This is to ensure that quadrilateral elements and triangular elements that might otherwise share the same region ID are mapped to separate block names. If [!param](/Mesh/PinMeshGenerator/use_as_assembly) is set to true, the block name will have the prefix `RGMB_ASSEMBLY<pin_type_id>` instead of `RGMB_PIN<pin_type_id>`.

## Reporting ID Information

As mentioned above, the `PinMeshGenerator` object tags the mesh elements with the extra integer reporting ID named "region_id".

The `PinMeshGenerator` object also automatically tags the mesh with the [!param](/Mesh/PinMeshGenerator/pin_type) using the extra integer name "pin_type_id" and, if extruded, the axial layers using the extra integer name "plane_id". If [!param](/Mesh/PinMeshGenerator/use_as_assembly) is set to true, the extra integer name "assembly_type_id" will be generated with integer values equivalent to "pin_type_id".

## Exterior Boundary ID Information

The `PinMeshGenerator` object automatically assigns boundary information derived from the [!param](/Mesh/PinMeshGenerator/pin_type) parameter. The exterior pin boundary is assigned the ID equal to 20000 + the pin type ID and is named "outer_pin_<pin_type_id>" (for example a pin with a pin type ID of 1 will have a boundary ID of 20001 and boundary name of "outer_pin_1"). If [!param](/Mesh/PinMeshGenerator/use_as_assembly) is set to true, the outer boundary name will be "outer_assembly_<pin_type_id>".

If the pin is extruded to three dimensions the top-most boundary ID must be assigned using [!param](/Mesh/ReactorMeshParams/top_boundary_id) and will have the name "top", while the bottom-most boundary must be assigned using [!param](/Mesh/ReactorMeshParams/bottom_boundary_id) and will have the name "bottom".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/pin_mesh_generator/pin_only.i block=Mesh

This is the resulting mesh block layout, where by default a single block is assigned to the triangular elements and another block is assigned to the quadrilateral elements:

!media reactor/meshgenerators/pin_mesh_generator.png style=width:40%;

This is the resulting "region_id" extra element integer layout, which was chosen by setting the region IDs for each radial region within the pin:

!media reactor/meshgenerators/pin_mesh_generator_rid.png style=width:40%;

!syntax parameters /Mesh/PinMeshGenerator

!syntax inputs /Mesh/PinMeshGenerator

!syntax children /Mesh/PinMeshGenerator
