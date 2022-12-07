# HeatStructurePlate

This component is a [2D heat structure](component_groups/heat_structure_2d.md)
that has a plate/brick/box shape.

## Usage

!template load file=heat_structure_usage.md.template name=HeatStructurePlate

!template load file=geometrical_component_usage.md.template name=HeatStructurePlate

!template load file=heat_structure_2d_usage.md.template name=HeatStructurePlate

The component is 2D but has a finite depth in the third dimension, which is
specified via [!param](/Components/HeatStructurePlate/depth).

!syntax parameters /Components/HeatStructurePlate

## Mesh id=mesh

### Axial Discretization id=mesh_axial

!template load file=geometrical_component_mesh.md.template name=HeatStructurePlate

### Radial Discretization id=mesh_radial

!template load file=heat_structure_2d_mesh_radial.md.template name=HeatStructurePlate

### Blocks and Boundaries id=mesh_blocks

!template load file=heat_structure_2d_mesh_blocks.md.template name=HeatStructurePlate

## Variables

!include heat_structure_variables.md

## Formulation

!include heat_structure_formulation.md

!syntax inputs /Components/HeatStructurePlate

!syntax children /Components/HeatStructurePlate
