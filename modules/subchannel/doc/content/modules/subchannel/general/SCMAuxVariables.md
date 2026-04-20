# SCMAuxVariables

Depending on the mesh type defined in the SCM input file, SCM automatically creates and populates the following auxiliary variables on the mesh. These variables are defined at equally spaced nodes from the inlet to the outlet. A subchannel cell spans the distance between two consecutive nodes.

## SubChannel mesh

This is the Subchannel mesh that gets created by: [SCMQuadSubChannelMeshGenerator](SCMQuadSubChannelMeshGenerator.md) and [SCMTriSubChannelMeshGenerator](SCMTriSubChannelMeshGenerator.md) for quadrilateral and triangular lattice respectively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the subchannel mesh block, automatically defines the following auxiliary variables that live on the subchannel mesh nodes:

- Mass flow rate: "mdot" $kg/s$

- Subchannel surface area: "S" $m^2$

- Subchannel wetted perimeter: "w_perim" $m$

- Net crossflow per subchannel cell: "SumWij" $kg/s$ (This refers to the cell value immediately below the node)

- Pressure: "P" $Pa$

- Enthalpy: "h" $J/kg$

- Temperature: "T" $K$

- Density: "rho" $kg/m^3$

- Viscosity: "mu" $Pa*s$

- Linear heat rate: "q_prime" $W/m$

- Subchannel centroid displacement: "displacement" $m$

An example usage of displacement can be found [here](areva_fctf.md).

### Flags

Enabling the boolean flag [!param](/Problem/QuadSubChannel1PhaseProblem/full_output) in the `[SubChannel]` problem creates the additional auxvariables:

- Pressure drop: "DP" $Pa$ (This refers to the cell value immediately below the node)

- Friction factor: "ff" $unitless$ (This refers to the cell value immediately below the node)

!alert note
This `DP` auxvariable variable is not calculated for the monolithic solve, in which case it will read zero in all nodes.

## Fuel pins mesh

This is the fuel pin mesh that gets created by: [SCMQuadPinMeshGenerator](SCMQuadPinMeshGenerator.md) and [SCMTriPinMeshGenerator](SCMTriPinMeshGenerator.md) for quadrilateral and triangular lattice respectively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the fuel pin mesh block, automatically defines the following auxiliary variables that live on the fuel pin mesh nodes:

- Fuel pin average surface temperature: $K$ = "Tpin"

- Fuel pin diameter: "Dpin" $m$

- Linear heat rate: "q_prime" $W/m$

- Average subchannel convective heat transfer coefficient: "HTC" $W/m^2K$

!alert note
The variable `q_prime` is created on: the fuel pin mesh if a pin mesh is present otherwise on the subchannel mesh

!alert note
In general, the pin diameter is defined by the mesh and provided by the user through the mesh generator objects. As a result, it is assumed to be constant for all fuel pins. If a pin mesh is present, the auxiliary variable `Dpin` is automatically created. Typically, the user must initialize `Dpin` to the pin diameter specified in the mesh generators [!param](/Mesh/SCMTriSubChannelMeshGenerator/pin_diameter), [!param](/Mesh/SCMQuadSubChannelMeshGenerator/pin_diameter), to avoid confusion and ensure consistency. This enforces the assumption that all fuel pins share the same diameter. The initial conditions used to compute geometric quantities—such as flow area and wetted perimeter—also rely on this assumption of uniform pin diameter and use pin diameter value from the mesh. If different pin diameters are required, the user can override this behavior by manually defining `Dpin` using a custom initial condition. In that case, the solver detects the variation and recomputes the geometric properties (surface area, wetted perimeter, and gap) before each time step.

## Duct mesh

This is the duct mesh that gets created by: [SCMQuadDuctMeshGenerator](SCMQuadDuctMeshGenerator.md) and [SCMTriDuctMeshGenerator](SCMTriDuctMeshGenerator.md) for square and triangular assemblies respetively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the duct mesh block, automatically defines the following auxiliary variables that live on the duct mesh nodes:

- Duct heat flux: "duct_heat_flux" $W/m^2$

- Duct temperature: "Tduct" $K$

!alert warning
 All ICs, AuxKernels, and Postprocessors using these variables must be applied on compatible blocks. Mismatched block names will result in an error.

!alert note
All SCM auxiliary variables are block-restricted by default. Each variable is defined only on the mesh block where it is physically meaningful (e.g., subchannel, fuel pins, or duct). When visualized in ParaView, these variables may appear across the entire domain (all mesh types). However, values outside their native blocks are not meaningful and may be shown as zero or interpolated by the visualization tool. For example, subchannel quantities such as surface area or mass flow rate are defined on the subchannel mesh and should only be interpreted there.
