# SCMAuxVariables

Depending on the mesh type defined in the SCM input file, SCM automatically creates and populates the following auxiliary variables on the mesh. These variables are defined at equally spaced nodes from the inlet to the outlet. A subchannel cell spans the distance between two consecutive nodes.

## SubChannel mesh

This is the subchannel mesh that gets created by: [SCMQuadAssemblyMeshGenerator](SCMQuadAssemblyMeshGenerator.md) and [SCMTriAssemblyMeshGenerator](SCMTriAssemblyMeshGenerator.md) for quadrilateral and triangular lattice respectively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the subchannel mesh block, automatically defines the following auxiliary variables that live on the subchannel mesh nodes:

- Mass flow rate: "mdot" $kg/s$

- Subchannel surface area: "S" $m^2$

- Subchannel wetted perimeter: "w_perim" $m$

- Net crossflow per subchannel cell: "SumWij" $kg/s$ (This refers to the cell value immediately below the node)

- Relative pressure: "P" $Pa$. This is the local pressure relative to the outlet pressure,
  $P = P_{absolute} - P_{out}$, so `P` is zero at the outlet. The solver adds the
  [!param](/Problem/QuadSubChannel1PhaseProblem/P_out) value back when evaluating fluid
  properties that require absolute pressure.

- Enthalpy: "h" $J/kg$

- Temperature: "T" $K$

- Density: "rho" $kg/m^3$

- Viscosity: "mu" $Pa*s$

- Linear heat rate: "q_prime" $W/m$

- Subchannel centroid displacement: "displacement" $m$

An example usage of displacement can be found [here](areva_fctf.md).

### Geometric auxiliary variable initialization

When the `[SubChannel]` problem syntax is present, SCM also adds default initial conditions for the geometry auxiliary variables. The subchannel flow area `S` is initialized by [SCMQuadFlowAreaIC.md](SCMQuadFlowAreaIC.md) or [SCMTriFlowAreaIC.md](SCMTriFlowAreaIC.md), and the subchannel wetted perimeter `w_perim` is initialized by [SCMQuadWettedPerimIC.md](SCMQuadWettedPerimIC.md) or [SCMTriWettedPerimIC.md](SCMTriWettedPerimIC.md), depending on whether the input uses a quadrilateral or triangular subchannel mesh. These initial conditions compute undeformed geometry from the corresponding subchannel mesh.

If a pin mesh is present, SCM also initializes `Dpin` with a constant value equal to the pin diameter specified by the subchannel mesh generator. These default initial conditions are only added when no initial condition has already been provided for the same variable. User-provided initial conditions, including custom initial condition kernels, take precedence over the SCM defaults.

The default `S` and `w_perim` values represent the mesh-generator geometry. If `Dpin` or `displacement` is initialized to values that differ from the undeformed mesh geometry, the solver detects the deformation and recomputes the subchannel flow area, wetted perimeter, and gap before solving.

### Flags

Enabling the boolean flag [!param](/Problem/QuadSubChannel1PhaseProblem/full_output) in the `[SubChannel]` problem creates the additional auxvariables:

- Pressure drop: "DP" $Pa$ (This refers to the cell value immediately below the node)

- Friction factor: "ff" $unitless$ (This refers to the cell value immediately below the node)

!alert note
This `DP` auxvariable variable is not calculated for the monolithic solve, in which case it will read zero in all nodes.

## Fuel pins mesh

This is the fuel pin mesh that gets created by: [SCMQuadAssemblyMeshGenerator](SCMQuadAssemblyMeshGenerator.md) and [SCMTriAssemblyMeshGenerator](SCMTriAssemblyMeshGenerator.md) for quadrilateral and triangular lattice respectively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators automatically defines the following auxiliary variables that live on the fuel pin mesh nodes when the assembly contains fuel pins:

- Fuel pin average surface temperature: "Tpin" $K$

- Fuel pin diameter: "Dpin" $m$

- Linear heat rate: "q_prime" $W/m$

- Average subchannel convective heat transfer coefficient: "HTC" $W/m^2K$

!alert note
The variable `q_prime` is created on: the fuel pin mesh if a pin mesh is present otherwise on the subchannel mesh

!alert note
In general, the pin diameter is defined by the mesh and provided by the user through the mesh generator objects [!param](/Mesh/SCMTriAssemblyMeshGenerator/pin_diameter), [!param](/Mesh/SCMQuadAssemblyMeshGenerator/pin_diameter). As a result, it is assumed to be constant for all fuel pins. If a pin mesh is present, the auxiliary variable `Dpin` is automatically created and initialized to the mesh-generator pin diameter. This enforces the assumption that all fuel pins share the same diameter. If different pin diameters are required, the user can override this behavior by manually defining `Dpin` using a custom initial condition. In that case, the solver detects the variation and recomputes the geometric properties (surface area, wetted perimeter, and gap) before each time step.

## Duct mesh

This is the duct mesh that gets created by: [SCMQuadDuctMeshGenerator](SCMQuadDuctMeshGenerator.md) and [SCMTriDuctMeshGenerator](SCMTriDuctMeshGenerator.md) for square and triangular assemblies respetively.

When the `[SubChannel]` problem syntax is present, defining either of these mesh generators in the duct mesh block, automatically defines the following auxiliary variables that live on the duct mesh nodes:

- Duct heat flux: "duct_heat_flux" $W/m^2$

- Duct temperature: "Tduct" $K$

!alert warning
 All ICs, AuxKernels, and Postprocessors using these variables must be applied on compatible blocks. Mismatched block names will result in an error.

## Auxiliary variables, block restrictions and output

All SCM auxiliary variables are block-restricted by default. Each variable is defined only on the mesh block where it is physically meaningful (e.g., subchannel, fuel pins, or duct). When visualized in ParaView, these variables may appear across the entire domain (all mesh types). However, values outside their native blocks are not meaningful and may be shown as zero or interpolated by the visualization tool. For example, subchannel quantities such as surface area or mass flow rate are defined on the subchannel mesh and should only be interpreted there.
