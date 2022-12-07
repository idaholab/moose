# HeatTransferFromHeatFlux1Phase

This component is a
[single-phase heat transfer component](thermal_hydraulics/component_groups/heat_transfer_1phase.md)
that uses a wall heat flux from a user-provided function.

## Usage

!template load file=heat_transfer_usage.md.template name=HeatTransferFromHeatFlux1Phase

!template load file=heat_transfer_1phase_usage.md.template name=HeatTransferFromHeatFlux1Phase

The parameter [!param](/Components/HeatTransferFromHeatFlux1Phase/q_wall)
specifies the wall heat flux function $q_\text{wall}$.

!syntax parameters /Components/HeatTransferFromHeatFlux1Phase

## Formulation

!include heat_transfer_1phase_formulation.md

!syntax inputs /Components/HeatTransferFromHeatFlux1Phase

!syntax children /Components/HeatTransferFromHeatFlux1Phase
