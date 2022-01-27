# HeatRateRadiationRZ

This post-processor computes the heat rate $\dot{Q}$ of a boundary radiative heat flux
applied to a cylindrical heat structure via the [HSBoundaryRadiation.md]
component:
\begin{equation}
  \dot{Q} = \int\limits_A \sigma \epsilon (T_{amb}^4 - T^4) f dA \,,
\end{equation}
where

- $\sigma$ is the Stefan-Boltzmann constant,
- $\epsilon$ is the emissivity,
- $T_{amb}$ is the ambient temperature,
- $T$ is the temperature of the surface, and
- $f$ is the view factor function.

!syntax parameters /Postprocessors/HeatRateRadiationRZ

!syntax inputs /Postprocessors/HeatRateRadiationRZ

!syntax children /Postprocessors/HeatRateRadiationRZ
