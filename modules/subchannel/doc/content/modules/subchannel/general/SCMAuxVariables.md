# SCMAuxVariables

Depending on the mesh type defined in the SCM input file, SCM automatically creates and populates the following auxiliary variables on the mesh. These variables are defined at equally spaced nodes from the inlet to the outlet. A subchannel cell spans the distance between two consecutive nodes.

## SubChannel mesh

This is the Subchannel mesh that gets created by: [SCMQuadSubChannelMeshGenerator](SCMQuadSubChannelMeshGenerator.md) and [SCMTriSubChannelMeshGenerator](SCMTriSubChannelMeshGenerator.md) for quadrilateral and triangular lattice respectively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the `[sub_channel]` mesh block, automatically defines the following auxiliary variables:

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

!alert note
The subchannel mesh block needs to be named: `[sub_channel]`.

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

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the `[fuel_pins]` mesh block, automatically defines the following auxiliary variables:

Fuel pin average surface temperature: $K$ = "Tpin"

Fuel pin diameter: $m$ = "Dpin"

Linear heat rate: $W/m$ = "q_prime"

The variable `q_prime` is created on:

- the `fuel_pins` mesh if a pin mesh is present

- otherwise on the `sub_channel` mesh

Average subchannel convective heat transfer coefficient: $W/m^2K$ = "HTC"

!alert note
The fuel pin mesh block needs to be named: `[fuel_pins]`.

## Duct mesh

This is the Duct mesh that gets created by: [SCMQuadDuctMeshGenerator](SCMQuadDuctMeshGenerator.md) and [SCMTriDuctMeshGenerator](SCMTriDuctMeshGenerator.md) for square and triangular assemblies respetively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the `[duct]` mesh block, automatically defines the following auxiliary variables:

Duct heat flux: $W/m^2$ = "duct_heat_flux"

Duct temperature: $K$ = "Tduct"

!alert note
The duct mesh block needs to be named: `[duct]`.

!alert warning
Block names must match those defined in the mesh. All ICs, AuxKernels, and Postprocessors using these variables must be applied on compatible blocks. Mismatched block names will result in an error.

!alert note
All SCM auxiliary variables are block-restricted by default. Each variable is defined only on the mesh block where it is physically meaningful (e.g., subchannel, fuel pins, or duct). When visualized in ParaView, these variables may appear across the entire domain. However, values outside their native blocks are not meaningful and may be shown as zero or interpolated by the visualization tool. For example, subchannel quantities such as surface area or mass flow rate are defined on the `[sub_channel]` mesh and should only be interpreted there.
