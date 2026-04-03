# SCMAuxVariables

Depending of the mesh type defined in the SCM input file, SCM automatically creates/populates the following auxvariables on the the mesh.

## SubChannelMesh

This is the Subchannel mesh that gets created by: [SCMQuadSubChannelMeshGenerator](SCMQuadSubChannelMeshGenerator.md) and [SCMTriSubChannelMeshGenerator](SCMTriSubChannelMeshGenerator.md) for square and triangular assemblies respetively.

Defining this type of mesh automatically creates the following auxvariables:

MASS_FLOW_RATE = "mdot"

SURFACE_AREA = "S"

SUM_CROSSFLOW = "SumWij"

PRESSURE = "P"

ENTHALPY = "h"

TEMPERATURE = "T"

DENSITY = "rho"

VISCOSITY = "mu"

WETTED_PERIMETER = "w_perim"

LINEAR_HEAT_RATE = "q_prime" on the Subchannel Mesh

### Flags

- Enabling the boolean flag [!param](/Problem/QuadSubChannel1PhaseProblem/full_output)

creates the additional auxvariables:

PRESSURE_DROP = "DP"

FRICTION_FACTOR = "ff"

- Enabling the boolean flag [!param](/Problem/QuadSubChannel1PhaseProblem/deformation)

creates the additional auxvariable:

DISPLACEMENT = "displacement"

An example usage of displacement can be found [here](areva_fctf.md).

## PinMesh

This is the Pin mesh that gets created by: [SCMQuadPinMeshGenerator](SCMQuadPinMeshGenerator.md) and [SCMTriPinMeshGenerator](SCMTriPinMeshGenerator.md) for square and triangular assemblies respetively.

PIN_TEMPERATURE = "Tpin"

PIN_DIAMETER = "Dpin"

LINEAR_HEAT_RATE = "q_prime" on the Pin Mesh.

HEAT_TRANSFER_COEFFICIENT = "HTC"

## DuctMesh

This is the Duct mesh that gets created by: [SCMQuadDuctMeshGenerator](SCMQuadDuctMeshGenerator.md) and [SCMTriDuctMeshGenerator](SCMTriDuctMeshGenerator.md) for square and triangular assemblies respetively.

DUCT_HEAT_FLUX = "duct_heat_flux"

DUCT_TEMPERATURE = "Tduct"
