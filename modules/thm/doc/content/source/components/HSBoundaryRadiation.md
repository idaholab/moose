# HSBoundaryRadiation

This component applies radiative heat transfer boundary conditions to a heat
structure boundary:
\begin{equation}
  q = \sigma \epsilon G (T^4 - T^4_\text{amb}) \,,
\end{equation}
where

- $\sigma$ is the Stefan-Boltzmann constant,
- $\epsilon$ is the emissivity of the surface,
- $G$ is the view factor function,
- $T$ is the temperature of the surface, and
- $T_\text{amb}$ is the temperature of the environment.

!syntax parameters /Components/HSBoundaryRadiation

!syntax inputs /Components/HSBoundaryRadiation

!syntax children /Components/HSBoundaryRadiation

!bibtex bibliography
