# Heat Structure Heat Source

These components provide volumetric heat sources to
[heat structures](thermal_hydraulics/component_groups/heat_structure.md).

The following components fall into this category:

- [HeatSourceFromPowerDensity.md]
- [HeatSourceFromTotalPower.md]

## Usage

The user must supply the name of the
[heat structure](thermal_hydraulics/component_groups/heat_structure.md) via the
parameter `hs` and then the applicable regions of the heat structure using the
`regions` parameter. For a
[2D heat structure](thermal_hydraulics/component_groups/heat_structure_2d.md),
`regions` may include any set of the heat structure's `names` parameter.
For [HeatStructureFromFile3D.md], `regions` may include any set of blocks
existing in the mesh file.

## Formulation

A volumetric heat source $q$ is added to the heat conduction equation as follows
for the domain $\Omega$ indicated by the `hs` and `regions` parameters:

!equation
\rho c_p \pd{T}{t} - \nabla \cdot (k \nabla T) = q \qquad \mathbf{r} \in \Omega
