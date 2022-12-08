# HeatTransferFromHeatStructure3D1Phase

This component is both a
[single-phase heat transfer component](thermal_hydraulics/component_groups/heat_transfer_1phase.md)
and a [heat structure boundary](thermal_hydraulics/component_groups/heat_structure_boundary.md).
It specifies a convective heat exchange between a [HeatStructureFromFile3D.md]
and one or more [FlowChannel1Phase.md] components.

## Usage

The parameter [!param](/Components/HeatTransferFromHeatStructure3D1Phase/flow_channels)
specifies the flow channels to connect. These flow channels must be aligned with
one of the axes (x, y, or z). The number of elements of the heat structure in
the direction of the flow channels must match the number of elements of the flow
channels, and the axial locations of the element centroids must match. The user
should check the flow channel and heat structure elements are properly aligned
to ensure the coupling is properly executed. No internal check is performed.

The parameter [!param](/Components/HeatTransferFromHeatStructure3D1Phase/hs)
specifies the name of the 3D heat structure, and the parameter
[!param](/Components/HeatTransferFromHeatStructure3D1Phase/boundary) specifies
the heat structure boundaries to connect to the flow channels.

!template load file=heat_transfer_1phase_usage.md.template name=HeatTransferFromHeatStructure3D1Phase

The parameter [!param](/Components/HeatTransferFromHeatStructure3D1Phase/P_hf)
is optional and specifies the heated perimeter $P_\text{heat}$; if unspecified,
this is computed from the cross-sectional area assuming a circular cross
section. To ensure energy conservation, the heated perimeter should be
calculated on the discretized heat structure boundary.

!syntax parameters /Components/HeatTransferFromHeatStructure3D1Phase

## Formulation

This component implements a convective heat exchange between the flow channel
and heat structure, with the flow channel receiving the following wall heat
flux:

!equation
q_\text{wall} = \mathcal{H}(T_s - T) \eqc

where $\mathcal{H}$ is the heat transfer coefficient, $T_s$ is the heat
structure surface temperature, and $T$ is the fluid temperature. On the heat
structure side, the incoming boundary flux is the opposite of that going into
the flow channel:

!equation
q_b = -q_\text{wall} = \mathcal{H}(T - T_s) \eqp

!syntax inputs /Components/HeatTransferFromHeatStructure3D1Phase

!syntax children /Components/HeatTransferFromHeatStructure3D1Phase
