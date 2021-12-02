# HexIDPatternedMeshGenerator

!syntax description /Mesh/HexIDPatternedMeshGenerator

## Overview


The `HexIDPatternedMeshGenerator` object generates a 2D Hexagonal lattice mesh with `reporting ID` assignments.
This object inherits the functionality of the lattice mesh generator named [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md) that stitches together Hexagonal cells, and adds additional functionality to assign reporting IDs to lattice cells.
The object can be used successively on its own output mesh to add IDs on the pin and assembly levels, for example.

## Reporting ID Information

The `HexIDPatternedMeshGenerator` object adopts the existing input structures of [`PatternedHexMeshGenerator`](PatternedHexMeshGenerator.md) for geometry building and uses additional keywords to control the reporting ID assignment.
A user can select an ID assignment scheme using [!param](/Mesh/HexIDPatternedMeshGenerator/assign_type), and the following schemes are currently available:

- `cell` (default):  Assign unique IDs for each component in the lattice in sequential order.

- `pattern`:  Assign IDs based on the ID of the input tiles.

- `manual`: Assign IDs based on user-defined mapping defined in [!param](/Mesh/HexIDPatternedMeshGenerator/id_pattern).

The default numbering scheme starts at 0 in the upper left hand corner of the hexagon grid (not including duct region) and increments by 1 as the grid is traversed left to right, top to bottom.
In presence of duct regions, separate reporting IDs are automatically generated for the elements on duct regions.
The duct regions will be assigned reporting IDs starting from the next integer higher than the highest one used inside of the ducts.

The name of the reporting ID variable is provided through [!param](/Mesh/HexIDPatternedMeshGenerator/id_name) depending on the hierarchical level of component.
The ID values themselves are stored as extra element integers on the mesh.
For example, the reporting IDs for individual pins (`pin_id`) can be assigned when assemblies are built because the IDs for pin level are uniquely determined from the pin arrangement within each assembly type.
Similarly, the assembly reporting IDs (`assembly_id`) are assigned in the core construction process.

Certain regions can be excluded from being labeled with an ID, for example dummy regions that will later be deleted.
This can be accommodated by listing mesh objects in the [!param](/Mesh/HexIDPatternedMeshGenerator/exclude_id) input parameter.
IDs will not be assigned to these mesh objects.

Usage of this parameter is helpful to retain sequential numbering when dummy region are later deleted, or to only label areas of interest.

## Example Syntax

In this example, the `HexIDPatternedMeshGenerator` is used to generate
an hexagonal assembly with pin reproting IDs (`pin_id`).

!listing modules/reactor/test/tests/meshgenerators/reporting_id/hexagonal_id/assembly_reporting_id.i block=Mesh/assembly

!syntax parameters /Mesh/HexIDPatternedMeshGenerator

!syntax inputs /Mesh/HexIDPatternedMeshGenerator

!syntax children /Mesh/HexIDPatternedMeshGenerator

