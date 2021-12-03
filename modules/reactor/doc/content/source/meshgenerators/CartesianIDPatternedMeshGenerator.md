# CartesianIDPatternedMeshGenerator

!syntax description /Mesh/CartesianIDPatternedMeshGenerator

## Overview

The `CartesianIDPatternedMeshGenerator` object generates a 2D Cartesian lattice mesh with `reporting ID` assignments.
This object inherits the functionality of the lattice mesh generator named [`PatternedMeshGenerator`](PatternedMeshGenerator.md) that stitches together Cartesian cells, and adds additional functionality to assign reporting IDs to lattice cells.
The object can be used successively on its own output mesh to add IDs on the pin and assembly levels, for example.

## Reporting ID Information

The `CartesianIDPatternedMeshGenerator` object adopts the existing input structures of [`PatternedMeshGenerator`](PatternedMeshGenerator.md) for geometry building and uses additional keywords to control the reporting ID assignment.

A user can select an ID assignment scheme using [!param](/Mesh/CartesianIDPatternedMeshGenerator/assign_type), and the following schemes are currently available:

- `cell` (default):  Assign unique  IDs for each component in the lattice in sequential order.

- `pattern`:  Assign IDs based on the ID of the input tiles.

- `manual`: Assign IDs based on user-defined mapping defined in [!param](/Mesh/CartesianIDPatternedMeshGenerator/id_pattern).

The default numbering scheme starts at 0 in the upper left hand corner of the grid and increments by 1 as the grid is traversed left to right, top to bottom.
The name of the reporting ID is provided through [!param](/Mesh/CartesianIDPatternedMeshGenerator/id_name) depending on the hierarchical level of component.
For example, the reporting IDs for individual pins (`pin_id`) can be assigned when assemblies are built, because the IDs for pin level are uniquely determined from the pin arrangement within each assembly type.
The ID values themselves are stored as extra element integers on the mesh.
Similarly, the assembly reporting IDs (`assembly_id`) are assigned in the core construction process.


Certain regions can be excluded from being labeled with an ID, for example dummy regions that will later be deleted.
This can be accommodated by listing mesh objects in the [!param](/Mesh/CartesianIDPatternedMeshGenerator/exclude_id) input parameter.
IDs will not be assigned to these mesh objects.
Usage of this parameter is helpful to retain sequential numbering when dummy region are later deleted, or to only label areas of interest.

## Example Syntax

In this example, the `CartesianIDPatternedMeshGenerator` is used to generate
a Cartesian assembly with pin reproting IDs (`pin_id`).

!listing modules/reactor/test/tests/meshgenerators/reporting_id/cartesian_id/assembly_reporting_id.i block=Mesh/assembly

!syntax parameters /Mesh/CartesianIDPatternedMeshGenerator

!syntax inputs /Mesh/CartesianIDPatternedMeshGenerator

!syntax children /Mesh/CartesianIDPatternedMeshGenerator

