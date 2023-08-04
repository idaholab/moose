# HeatTransferFromExternalAppHeatFlux1Phase

!syntax description /Components/HeatTransferFromExternalAppHeatFlux1Phase

This component is a
[single-phase heat transfer component](thermal_hydraulics/component_groups/heat_transfer_1phase.md).

It adds a [ADOneDEnergyWallHeatFlux.md] kernel to the energy equation. The
heat source is gathered from a constant monomial variable named "q_wall" plus a suffix dependent on
the index of the connected heat transfer component. This variable should
be set using transferred data from an external application.

The variable is provided to the [ADOneDEnergyWallHeatFlux.md] kernel as a material property.
The conversion is done using a [CoupledVariableValueMaterial.md].

## Formulation

Please refer to [ADOneDEnergyWallHeatFlux.md] for the heat flux formulation.

!syntax parameters /Components/HeatTransferFromExternalAppHeatFlux1Phase

!syntax inputs /Components/HeatTransferFromExternalAppHeatFlux1Phase

!syntax children /Components/HeatTransferFromExternalAppHeatFlux1Phase
