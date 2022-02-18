# HeatStructureFromFile3D

This component allows users to load a 3D mesh from a file into a
[heat structure](thermal_hydraulics/component_groups/heat_structure.md).

The block, sideset, and nodeset names are preserved, but are prepended with a component name. For example,
if the component name is `hs` and the mesh has a block called `body`, users can refer to this block
using the `hs:body` name, and similarly for sidesets and nodesets.

## Usage

!template load file=heat_structure_usage.md.template name=HeatStructureFromFile3D

The following parameters need to be specified:

- [!param](/Components/HeatStructureFromFile3D/position): The location in 3D
  space where the origin of the mesh will be placed. This can be used to move
  the mesh from its original location specified in the file.
- [!param](/Components/HeatStructureFromFile3D/file): the ExodusII file name
  with the mesh to load. Note that ExodusII is the only supported format at the
  moment.

To define the material properties used by the heat conduction model, users must
create materials that define the following AD material properties on all of
their blocks (see [#mesh]):

| Material Property | Symbol | Description |
| :- | :- | :- |
| `density` | $\rho$ | Density \[kg/m$^3$\] |
| `specific_heat` | $c_p$ | Specific heat capacity \[J/(kg-K)\] |
| `thermal_conductivity` | $k$ | Thermal conductivity \[W/(m-K)\] |

!syntax parameters /Components/HeatStructureFromFile3D

## Mesh id=mesh

The block and sideset names in the loaded mesh file are prepended with the name
of the component. For example if the heat structure component name is `myhs`
and the mesh file has the block `myblock`, then the block `myhs:myblock` is
created.

## Variables

!include heat_structure_variables.md

## Formulation

!include heat_structure_formulation.md

!syntax inputs /Components/HeatStructureFromFile3D

!syntax children /Components/HeatStructureFromFile3D
