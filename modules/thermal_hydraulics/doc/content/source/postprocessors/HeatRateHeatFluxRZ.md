# HeatRateHeatFluxRZ

This post-processor computes the heat rate $\dot{Q}$ of a boundary heat flux function
applied to a cylindrical heat structure via the [HSBoundaryHeatFlux.md]
component:

!equation
\dot{Q} = \int\limits_A q dA \,,

where $q$ is the user-provided heat flux function.

!syntax parameters /Postprocessors/HeatRateHeatFluxRZ

!syntax inputs /Postprocessors/HeatRateHeatFluxRZ

!syntax children /Postprocessors/HeatRateHeatFluxRZ
