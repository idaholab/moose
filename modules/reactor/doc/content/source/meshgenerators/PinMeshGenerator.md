# PinMeshGenerator

!syntax description /Mesh/PinMeshGenerator

## Overview
This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

The `PinMeshGenerator` object generates square or hexagonal reactor geometry pin cell structures which may be combined into larger assembly structures using `AssemblyMeshGenerator`. The block IDs, external boundary ID, region IDs (e.g., materials), and reporting IDs (extra element integers identifying unique planes and pins) are automatically assigned once the user provides some basic information. The pin defined may be extruded to three dimensions ([!param](/Mesh/PinMeshGenerator/extrude) equals 'true'), however it cannot be used in further mesh definition with `AssemblyMeshGenerator`.
This object automates the use and functionality of the [`PolygonConcentricCircleMeshGenerator`](PolygonConcentricCircleMeshGenerator.md) and, if extruding to three dimension, the [`FancyExtruderGenerator'](FancyExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`TransformGenerator`](TransformGenerator.md), [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md), and [`PlaneIDMeshGenerator`](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment and boundary ID and name assignment. 
The `PinMeshGenerator` object adopts much of the existing input structure of `PolygonConcentricCircleMeshGenerator`](PolygonConcentricCircleMeshGenerator.md) but adopts the use of parameters that are more typical for reactor design. 

## Block ID Information
The [!param](/Mesh/PinMeshGenerator/region_ids) parameter is used to identify regions within the pin that belong to the same block both radially and axially. This functionality is intended for easy identification of regions within the mesh that will have the same properties, such as material assignments, and will assign the region ID to both the subdomain (block) ID and name.
This parameter can be directly input, or can be automated with the [!param](/Mesh/ReactorMeshParams/procedural_region_ids) in the [`ReactorMeshParams`](ReactorMeshParams.md) MeshGenerator input for [!param](/Mesh/PinMeshGenerator/reactor_params). If explicit assignment is being used it should be noted that if [!param](/Mesh/PinMeshGenerator/quad_center_elements) is false than the center most radial mesh division needs to be its own region due to element type restrictions for block IDs.
In the case of user-defined subdomain ID assignment, [!param](/Mesh/PinMeshGenerator/region_ids) should be given as a vector starting from the center-most region, or in the case of a three dimensional problem, a two dimensional array with each component vector being the IDs for the appropriate axial region starting from the bottom of the geometry. 
The automated ID assignment, via the [!param](/Mesh/ReactorMeshParams/procedural_region_ids) option, concatenates the [!param](/Mesh/PinMeshGenerator/pin_type) and the radial mesh region number (indexed starting at 0 for the innermost region). If radial regions are used, the innermost meshed interval will always be made a unique region to allow for either quad or tri center elements. In the case that the concatenatation exceeds the type limit, the lowest digits of the [!param](/Mesh/PinMeshGenerator/pin_type) will be truncated. The same ID will be used for all axial layers in three dimensional meshes.

For example a pin with a type ID of 5 with two rings a duct would procedurally generate four region IDs:
- The center-most ring (assuming it only has a single mesh interval) region ID of 50
- The second ring region ID of 51
- The background region ID of 52
- The duct region ID of 53

If the same pin instead has a type ID of 65535 the same four regions would be assigned as:
- The center-most ring region ID of 65530
- The second ring region ID of 65531
- The background region ID of 65532
- The duct region ID of 65533


## Reporting ID Information
The `PinMeshGenerator` object also tags the mesh elements with the reporting ID named "region_id".
The `PinMeshGenerator` object also automatically tags the mesh with the [!param](/Mesh/PinMeshGenerator/pin_type) using the name "pin_type_id" and, if extruded, the axial layers using the name "plane_id".

## Exterior Boundary ID Information
The `PinMeshGenerator` object automatically assigns boundary information derived from the [!param](/Mesh/PinMeshGenerator/pin_type) parameter. The exterior pin boundary is assigned the ID equal to 20000 + the pin type ID and is named "outer_pin_<pin_type_id>" (for example a pin with a pin type ID of 1 will have a boundary ID of 20001 and boundary name of "outer_pin_1").
If the core is extruded to three dimensions the top-most boundary will be assigned an ID equal to 201 and the name "top", while the bottom-most boundary will be assigned an ID equal to 202 and the name "bottom".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/pin_mesh_generator/pin_only_hex_complex.i block=Mesh/pin1

!syntax parameters /Mesh/PinMeshGenerator

!syntax inputs /Mesh/PinMeshGenerator

!syntax children /Mesh/PinMeshGenerator
