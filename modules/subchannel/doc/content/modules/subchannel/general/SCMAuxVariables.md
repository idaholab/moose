# SCMAuxVariables

Depending on the mesh type defined in the SCM input file, SCM automatically creates and populates the following auxiliary variables on the mesh. These variables are defined at equally spaced nodes from the inlet to the outlet. A subchannel cell spans the distance between two consecutive nodes.

## SubChannel mesh

This is the Subchannel mesh that gets created by: [SCMQuadSubChannelMeshGenerator](SCMQuadSubChannelMeshGenerator.md) and [SCMTriSubChannelMeshGenerator](SCMTriSubChannelMeshGenerator.md) for quadrilateral and triangular lattice respectively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators automatically defines the following auxiliary variables:

Mass flow rate: $kg/s$ = "mdot"

Subchannel surface area: $m^2$ = "S"

Subchannel wetted perimeter: $m$ = "w_perim"

Net crossflow per subchannel cell: $kg/s$ = "SumWij" (This applies to the cell immediately behind the node)

Pressure: $Pa$ = "P"

Enthalpy: $J/kg$ = "h"

Temperature: $K$ = "T"

Density: $kg/m^3$ = "rho"

Viscosity: $Pa*s$ = "mu"

Linear heat rate: $W/m$ = "q_prime"

### Flags

- Enabling the boolean flag [!param](/Problem/QuadSubChannel1PhaseProblem/full_output) in the `[SubChannel]` problem creates the additional auxvariables:

Pressure drop: $Pa$ = "DP" (This applies to the cell immediately behind the node)

!alert note
This `DP` auxvariable variable is not calculated for the monolithic solve, in which case it will read zero in all nodes.

Friction factor: $unitless$ = "ff" (This applies to the cell immediately behind the node)

- Enabling the boolean flag [!param](/Problem/QuadSubChannel1PhaseProblem/deformation) in the `[SubChannel]` problem creates the additional auxvariables:

Subchannel centroid displacement: $m$ = "displacement"

An example usage of displacement can be found [here](areva_fctf.md).

## Fuel pins mesh

This is the Pin mesh that gets created by: [SCMQuadPinMeshGenerator](SCMQuadPinMeshGenerator.md) and [SCMTriPinMeshGenerator](SCMTriPinMeshGenerator.md) for quadrilateral and triangular lattice respectively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators automatically defines the following auxiliary variables:

Fuel pin average surface temperature: $K$ = "Tpin"

Fuel pin diameter: $m$ = "Dpin"

Linear heat rate: $W/m$ = "q_prime"

Average subchannel convective heat transfer coefficient: $W/m^2K$ = "HTC"

## Duct mesh

This is the Duct mesh that gets created by: [SCMQuadDuctMeshGenerator](SCMQuadDuctMeshGenerator.md) and [SCMTriDuctMeshGenerator](SCMTriDuctMeshGenerator.md) for square and triangular assemblies respetively.

When the `[Subchannel]` problem syntax is present, defining either of these mesh generators automatically defines the following auxiliary variables:

Duct heat flux: $W/m^2$ = "duct_heat_flux"

Duct temperature: $K$ = "Tduct"

!alert note
All created SCM `auxvariables` will have values on all mesh nodes but will be sensible populated with values where it makes physical sense. For example: subchannel quantinties like surface area or mass flow rate will have non-zero values on the subchannel mesh and zero values on the fuel pin mesh or the duct mesh if those exist.
