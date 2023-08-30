# ADRadiativeHeatFluxBC

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

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HSBoundaryRadiation.md] to place a radiative heat flux boundary condition on a heat structure.

!syntax parameters /BCs/ADRadiativeHeatFluxBC

!syntax inputs /BCs/ADRadiativeHeatFluxBC

!syntax children /BCs/ADRadiativeHeatFluxBC
