# HeatTransferFromSpecifiedTemperature1Phase

This component is a
[single-phase heat transfer component](thermal_hydraulics/component_groups/heat_transfer_1phase.md)
that specifies a convective heating condition via a provided wall temperature.

## Usage

!template load file=heat_transfer_usage.md.template name=HeatTransferFromSpecifiedTemperature1Phase

!template load file=heat_transfer_1phase_usage.md.template name=HeatTransferFromSpecifiedTemperature1Phase

The parameter [!param](/Components/HeatTransferFromSpecifiedTemperature1Phase/T_wall)
specifies the wall temperature function $T_\text{wall}$.

!syntax parameters /Components/HeatTransferFromSpecifiedTemperature1Phase

## Formulation

!include heat_transfer_1phase_formulation.md

This component computes the wall heat flux as follows:

!equation
q_\text{wall} = \mathcal{H}(T_\text{wall} - T) \eqp

!syntax inputs /Components/HeatTransferFromSpecifiedTemperature1Phase

!syntax children /Components/HeatTransferFromSpecifiedTemperature1Phase
