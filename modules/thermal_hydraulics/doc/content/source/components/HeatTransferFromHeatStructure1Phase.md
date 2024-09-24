# HeatTransferFromHeatStructure1Phase

This component is both a
[single-phase heat transfer component](thermal_hydraulics/component_groups/heat_transfer_1phase.md)
and a [heat structure boundary](thermal_hydraulics/component_groups/heat_structure_boundary.md).
It specifies a convective heat exchange between a
[FlowChannel1Phase.md] and a [2D heat structure](thermal_hydraulics/component_groups/heat_structure_2d.md).

## Usage

!template load file=heat_transfer_usage.md.template name=HeatTransferFromHeatStructure1Phase

!template load file=heat_transfer_1phase_usage.md.template name=HeatTransferFromHeatStructure1Phase

The parameter [!param](/Components/HeatTransferFromHeatStructure1Phase/hs) specifies
the name of the connected heat structure, and
[!param](/Components/HeatTransferFromHeatStructure1Phase/hs_side) specifies the
side of the connected heat structure that is coupled to the flow channel.

!alert note title=Flow channel alignment
The flow channel axis must be parallel to the heat structure axis and have
the same discretization along their axes.

The parameter [!param](/Components/HeatTransferFromHeatStructure1Phase/scale) specifies
the name of a [functor](Functors/index.md) $f$ that can scale the heat flux, for
example, a functor material property created with [FinEnhancementFactorFunctorMaterial.md]
for heat transfer enhancement due to fins.

!syntax parameters /Components/HeatTransferFromHeatStructure1Phase

## Formulation

This component implements a convective heat exchange between the flow channel
and heat structure, with the flow channel receiving the following wall heat
flux:

!equation
q_\text{wall} = f \mathcal{H}(T_s - T) \eqc

where $\mathcal{H}$ is the heat transfer coefficient, $T_s$ is the heat
structure surface temperature, $T$ is the fluid temperature, and $f$ is an optional scaling factor. On the heat
structure side, the incoming boundary flux is the opposite of that going into
the flow channel:

!equation
q_b = -q_\text{wall} = f \mathcal{H}(T - T_s) \eqp

!syntax inputs /Components/HeatTransferFromHeatStructure1Phase

!syntax children /Components/HeatTransferFromHeatStructure1Phase
