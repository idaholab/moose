# ADOneDEnergyWallHeating

!syntax description /Kernels/ADOneDEnergyWallHeating

The wall heating is expressed as a convection term in the energy equation strong form:

!equation
H_w P_{hf} (T_{fluid} - T_{wall})

where $H_w$ is the convective heat transfer coefficient, $P_{hf}$ is the heated perimeter,
$T_{fluid}$ is the single-phase fluid temperature and $T_{wall}$ is the wall temperature.
The wall temperature is provided as a field variable.

!alert note
In THM, most kernels are added automatically by components. This kernel is created by components derived from
`HeatTransferFromTemperature1Phase`, such as [HeatTransferFromSpecifiedTemperature1Phase.md], for modeling
wall heating in a 1-phase flow channel component.

!syntax parameters /Kernels/ADOneDEnergyWallHeating

!syntax inputs /Kernels/ADOneDEnergyWallHeating

!syntax children /Kernels/ADOneDEnergyWallHeating
