# HeatRateConvectionRZ

This post-processor computes the heat rate $\dot{Q}$ of a boundary convective heat flux
applied to a cylindrical heat structure via the [HSBoundaryAmbientConvection.md]
component:
\begin{equation}
  \dot{Q} = \int\limits_A h (T_{amb} - T) dA \,,
\end{equation}
where

- $h$ is the convective heat transfer coefficient,
- $T_{amb}$ is the ambient temperature, and
- $T$ is the temperature of the surface.

!syntax parameters /Postprocessors/HeatRateConvectionRZ

!syntax inputs /Postprocessors/HeatRateConvectionRZ

!syntax children /Postprocessors/HeatRateConvectionRZ
