# HeatStructureCylindrical

This component is a [2D heat structure](component_groups/heat_structure_2d.md)
that has axisymmetry; thus it is either a cylinder or cylindrical shell.

## Usage

!template load file=heat_structure_usage.md.template name=HeatStructureCylindrical

!template load file=geometrical_component_usage.md.template name=HeatStructureCylindrical

!template load file=heat_structure_2d_usage.md.template name=HeatStructureCylindrical

If the domain has some inner radius, then this is specified with
[!param](/Components/HeatStructureCylindrical/inner_radius); otherwise, it is
assumed to be a solid cylinder.

!syntax parameters /Components/HeatStructureCylindrical

## Mesh id=mesh

### Axial Discretization id=mesh_axial

!template load file=geometrical_component_mesh.md.template name=HeatStructureCylindrical

### Radial Discretization id=mesh_radial

!template load file=heat_structure_2d_mesh_radial.md.template name=HeatStructureCylindrical

### Blocks and Boundaries id=mesh_blocks

!template load file=heat_structure_2d_mesh_blocks.md.template name=HeatStructureCylindrical

## Variables

!include heat_structure_variables.md

## Formulation

!include heat_structure_formulation.md

!syntax inputs /Components/HeatStructureCylindrical

!syntax children /Components/HeatStructureCylindrical
