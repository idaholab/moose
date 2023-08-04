# HeatTransferFromExternalAppTemperature1Phase

!syntax description /Components/HeatTransferFromExternalAppTemperature1Phase

This component is a [single-phase heat transfer component](thermal_hydraulics/component_groups/heat_transfer_1phase.md).

It adds a [ADOneDEnergyWallHeating.md] kernel to the energy equation.
The wall temperature is added as either a constant monomial or a linear Lagrange variable, depending
on the [!param](/Components/HeatTransferFromExternalAppTemperature1Phase/var_type) parameter.
It is named after the [!param](/Components/HeatTransferFromExternalAppTemperature1Phase/T_ext) parameter.
This variable should be set using transferred data from an external application.

## Formulation

Please refer to [ADOneDEnergyWallHeating.md] for the heat flux formulation.

!syntax parameters /Components/HeatTransferFromExternalAppTemperature1Phase

!syntax inputs /Components/HeatTransferFromExternalAppTemperature1Phase

!syntax children /Components/HeatTransferFromExternalAppTemperature1Phase
