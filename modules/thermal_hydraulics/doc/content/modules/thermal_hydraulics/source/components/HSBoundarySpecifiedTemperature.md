# HSBoundarySpecifiedTemperature

This component is a heat structure boundary condition that applies Dirichlet
boundary conditions.

## Usage

The parameter [!param](/Components/HSBoundarySpecifiedTemperature/T) specifies
the temperature function $T_b$ to strongly impose on the boundary.

!syntax parameters /Components/HSBoundarySpecifiedTemperature

## Formulation

This boundary condition ensures the following:

!equation
T(\mathbf{r},t) = T_b(\mathbf{r},t) \qquad \mathbf{r} \in \Gamma

where

- $T$ is the temperature solution,
- $T_b$ is the imposed temperature function, and
- $\Gamma$ is the chosen boundary.

!syntax inputs /Components/HSBoundarySpecifiedTemperature

!syntax children /Components/HSBoundarySpecifiedTemperature
