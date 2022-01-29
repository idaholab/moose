# HSBoundaryAmbientConvection

This component applies convective heat transfer boundary conditions to a heat
structure boundary.

## Usage

The parameter [!param](/Components/HSBoundaryAmbientConvection/T_ambient) gives the ambient temperature $T_\infty$, and
[!param](/Components/HSBoundaryAmbientConvection/htc_ambient) gives the heat transfer coefficient $\mathcal{H}$.

!syntax parameters /Components/HSBoundaryAmbientConvection

## Formulation

The boundary heat flux $q_b$ is computed as

!equation
q_b = \mathcal{H} (T - T_\infty) \eqc

where

- $\mathcal{H}$ is the heat transfer coefficient,
- $T$ is the temperature of the surface, and
- $T_\infty$ is the ambient temperature.

!syntax inputs /Components/HSBoundaryAmbientConvection

!syntax children /Components/HSBoundaryAmbientConvection
