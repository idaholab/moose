# HeatRateConvection

This post-processor computes the heat rate $\dot{Q}$ of a boundary convective heat flux
applied to a plate or 3D heat structure via the [HSBoundaryAmbientConvection.md]
component:
\begin{equation}
  \dot{Q} = \int\limits_A h (T_{amb} - T) dA \,,
\end{equation}
where

- $h$ is the convective heat transfer coefficient,
- $T_{amb}$ is the ambient temperature, and
- $T$ is the temperature of the surface.

If the boundary corresponds to a [HeatStructurePlate.md], then the parameter
[!param](/Postprocessors/HeatRateConvection/scale) should be set to the value used for the
[!param](/Components/HeatStructurePlate/depth) parameter in the [HeatStructurePlate.md].
If the boundary corresponds to a [HeatStructureFromFile3D.md], then
[!param](/Postprocessors/HeatRateConvection/scale) should be omitted, which defaults
to a value of 1.

!syntax parameters /Postprocessors/HeatRateConvection

!syntax inputs /Postprocessors/HeatRateConvection

!syntax children /Postprocessors/HeatRateConvection
