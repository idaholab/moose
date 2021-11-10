# HeatStructureCylindrical

This component is a [2D heat structure](component_groups/heat_structure_2d.md)
that has axisymmetry; thus it is either a cylinder or cylindrical shell.

## Usage

If the domain has some inner radius, then this is specified with
[!param](/Components/HeatStructureCylindrical/inner_radius); otherwise, it is
assumed to be a solid cylinder.

!syntax parameters /Components/HeatStructureCylindrical

!syntax inputs /Components/HeatStructureCylindrical

!syntax children /Components/HeatStructureCylindrical
