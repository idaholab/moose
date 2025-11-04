# ConvectiveHeatFluxBC

!syntax description /BCs/ConvectiveHeatFluxBC

## Description

The `ConvectiveHeatFluxBC` boundary condition imposes a heat flux equal to

\begin{equation}
\vec{q}\cdot\hat{n}=h\left(T-T_\infty\right)
\end{equation}

where $\vec{q}\cdot\hat{n}$ is the heat flux normal to the boundary, $h$ is
the convective heat transfer coefficient, and $T_\infty$ is the far-field temperature.
Both $h$ and $T_\infty$ are taken as material properties.
See [CoupledConvectiveHeatFluxBC](CoupledConvectiveHeatFluxBC.md) for a similar boundary condition coupled to variables.

## Example Input File Syntax

!listing /convective_heat_flux/equilibrium.i block=BCs/right

!syntax parameters /BCs/ConvectiveHeatFluxBC

!syntax inputs /BCs/ConvectiveHeatFluxBC

!syntax children /BCs/ConvectiveHeatFluxBC
