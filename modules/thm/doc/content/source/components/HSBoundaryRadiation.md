# HSBoundaryRadiation

This component applies radiative heat transfer boundary conditions to a heat
structure boundary.

## Usage

The parameter [!param](/Components/HSBoundaryRadiation/T_ambient) gives the ambient temperature $T_\infty$,
[!param](/Components/HSBoundaryRadiation/emissivity) gives the surface emissivity $\epsilon$, and
[!param](/Components/HSBoundaryRadiation/view_factor) gives the view factor $F$.

!syntax parameters /Components/HSBoundaryRadiation

## Formulation

The boundary heat flux $q_b$ is computed as

\begin{equation}
  q_b = \sigma \epsilon F (T^4 - T^4_\infty) \eqc
\end{equation}
where

- $\sigma$ is the Stefan-Boltzmann constant,
- $\epsilon$ is the emissivity of the surface,
- $F$ is the view factor function,
- $T$ is the temperature of the surface, and
- $T_\infty$ is the ambient temperature.

!syntax inputs /Components/HSBoundaryRadiation

!syntax children /Components/HSBoundaryRadiation
