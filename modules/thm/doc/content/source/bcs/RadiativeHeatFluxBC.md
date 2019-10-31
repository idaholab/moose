# RadiativeHeatFluxBC

This boundary condition is used to supply a radiative heat flux:
\begin{equation}
  q = \sigma \epsilon G (T^4 - T^4_\text{amb}) \,,
\end{equation}
where

- $\sigma$ is the Stefan-Boltzmann constant,
- $\epsilon$ is the emissivity of the surface,
- $G$ is the view factor function,
- $T$ is the temperature of the surface, and
- $T_\text{amb}$ is the temperature of the environment.

!syntax parameters /BCs/RadiativeHeatFluxBC

!syntax inputs /BCs/RadiativeHeatFluxBC

!syntax children /BCs/RadiativeHeatFluxBC
