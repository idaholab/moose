# HeatRateRadiation

This post-processor computes the heat rate $\dot{Q}$ of a boundary radiative heat flux
applied to a plate or 3D heat structure via the [HSBoundaryRadiation.md]
component:
\begin{equation}
  \dot{Q} = \int\limits_A \sigma \epsilon (T_{amb}^4 - T^4) F dA \,,
\end{equation}
where

- $\sigma$ is the Stefan-Boltzmann constant,
- $\epsilon$ is the emissivity,
- $T_{amb}$ is the ambient temperature,
- $T$ is the temperature of the surface, and
- $F$ is the view factor function.

If the boundary corresponds to a [HeatStructurePlate.md], then the parameter
[!param](/Postprocessors/HeatRateRadiation/scale) should be set to the value used for the
[!param](/Components/HeatStructurePlate/depth) parameter in the [HeatStructurePlate.md].
If the boundary corresponds to a [HeatStructureFromFile3D.md], then
[!param](/Postprocessors/HeatRateRadiation/scale) should be omitted, which defaults
to a value of 1.

!syntax parameters /Postprocessors/HeatRateRadiation

!syntax inputs /Postprocessors/HeatRateRadiation

!syntax children /Postprocessors/HeatRateRadiation
