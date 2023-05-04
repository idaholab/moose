# HeatRateHeatFlux

This post-processor computes the heat rate $\dot{Q}$ of a boundary heat flux
function applied to a plate or 3D heat structure via the [HSBoundaryHeatFlux.md]
component:

!equation
\dot{Q} = \int\limits_A q dA \,,

where $q$ is the user-provided heat flux function.

If the boundary corresponds to a [HeatStructurePlate.md], then the parameter
[!param](/Postprocessors/HeatRateHeatFlux/scale) should be set to the value used for the
[!param](/Components/HeatStructurePlate/depth) parameter in the [HeatStructurePlate.md].
If the boundary corresponds to a [HeatStructureFromFile3D.md], then
[!param](/Postprocessors/HeatRateHeatFlux/scale) should be omitted, which defaults
to a value of 1.

!syntax parameters /Postprocessors/HeatRateHeatFlux

!syntax inputs /Postprocessors/HeatRateHeatFlux

!syntax children /Postprocessors/HeatRateHeatFlux
