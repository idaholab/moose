# HSBoundarySpecifiedTemperature

This component is a
[heat structure boundary](thermal_hydraulics/component_groups/heat_structure_boundary.md)
that applies Dirichlet boundary conditions.

## Usage

!template load file=heat_structure_boundary_usage.md.template name=HSBoundarySpecifiedTemperature

The parameter [!param](/Components/HSBoundarySpecifiedTemperature/T) specifies
the temperature function $T_b$ to strongly impose on the boundary.

!syntax parameters /Components/HSBoundarySpecifiedTemperature

## Formulation

!include heat_structure_boundary_formulation_dirichlet.md

!syntax inputs /Components/HSBoundarySpecifiedTemperature

!syntax children /Components/HSBoundarySpecifiedTemperature
