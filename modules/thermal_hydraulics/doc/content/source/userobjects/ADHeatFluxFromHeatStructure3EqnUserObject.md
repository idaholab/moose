# ADHeatFluxFromHeatStructure3EqnUserObject

!syntax description /UserObjects/ADHeatFluxFromHeatStructure3EqnUserObject

The heat flux $q$ is computed as:

!equation
q = H_{wall} (T_{wall} - T)

where $H_{wall}$ is the wall heat transfer coefficient, $T_{wall}$ is the wall temperature on the
heat structure side, and $T$ is the fluid temperature.

!alert note
This user object is created automatically by the [HeatTransferFromHeatStructure1Phase.md]
component, users do not need to add it to an input file.

!syntax parameters /UserObjects/ADHeatFluxFromHeatStructure3EqnUserObject

!syntax inputs /UserObjects/ADHeatFluxFromHeatStructure3EqnUserObject

!syntax children /UserObjects/ADHeatFluxFromHeatStructure3EqnUserObject
