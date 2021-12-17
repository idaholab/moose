# PinMeshGenerator

!syntax description /Mesh/PinMeshGenerator

## Overview

The `PinMeshGenerator` object generates pin-like reactor geometry structures in either square or hexagonal geometries with `reporting ID` and block ID assignments.
This object automates the use and functionality of the [`PolygonConcentricCircleMeshGenerator`](PolygonConcentricCircleMeshGenerator.md) and, if extruding to three dimension, the [`FancyExtruderGenerator'](FancyExtruderGenerator.md) through the use of the `MeshSubgenerator` functionality and supporting functionality from [`TransformGenerator`](TransformGenerator.md), [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md), and [`PlaneIDMeshGenerator`](PlaneIDMeshGenerator.md). In addition to the functionality of the `MeshGenerators` used, this object also automates block ID assignment and boundary ID and name assignment. The boundaries are assigned the ID equal to 20000+[!param](/Mesh/PinMeshGenerator/pin_type) and the boundary name is the concatenatation of "outer_pin_" and the [!param](/Mesh/PinMeshGenerator/pin_type). 
The `PinMeshGenerator` object adopts much of the existing input structure of `PolygonConcentricCircleMeshGenerator`](PolygonConcentricCircleMeshGenerator.md) but also adapts to use parameters that are more accessible for reactor design. 
This object is designed to be used in the Reactor MeshGenerator workflow, which also consists of [`ReactorMeshParams`](ReactorMeshParams.md), [`AssemblyMeshGenerator`](AssemblyMeshGenerator.md), and [`CoreMeshGenerator`](CoreMeshGenerator.md).

## Reporting ID Information

The `PinMeshGenerator` object allows for either user defined or automated ID assignments for material regions ([!param](/Mesh/PinMeshGenerator/region_ids)) that are assigned to the block IDs, block names, and element intergers with the tag "region_id".

The automated ID assignment concatenates the [!param](/Mesh/PinMeshGenerator/pin_type) and the radial mesh region number (indexed starting at 1 for the innermost region). In the case that the concatenatation exceeds the type limit, the lowest digits of the [!param](/Mesh/PinMeshGenerator/pin_type) will be truncated. The same ID will be used for all axial layers in three dimensional meshes.
The user defined ID assignment using [!param](/Mesh/PinMeshGenerator/region_ids) should be given as a vector starting from the center-most region, or in the case of a three dimensional problem, a two dimensional array with each component vector being the IDs for the appropriate axial region starting from the bottom of the geometry. 

The `PinMeshGenerator` object also automatically tags the mesh with the [!param](/Mesh/PinMeshGenerator/pin_type) using the name "pin_type_id" and, if extruded, the axial layers using the name "plane_id".

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/pin_mesh_generator/pin_only.i block=Mesh/pin1

!syntax parameters /Mesh/PinMeshGenerator

!syntax inputs /Mesh/PinMeshGenerator

!syntax children /Mesh/PinMeshGenerator
