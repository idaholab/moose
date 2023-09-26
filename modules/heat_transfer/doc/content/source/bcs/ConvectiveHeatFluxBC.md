# ConvectiveHeatFluxBC

!syntax description /BCs/ConvectiveHeatFluxBC

This boundary condition computes convective heat flux $q'' = H \cdot (T - T_{inf})$, where $H$ is convective heat transfer coefficient,
$T$ is the temperature, and $T_{inf}$ is far field temperature.  Both $H$ and $T_{inf}$ are coupled as material properties.
See [CoupledConvectiveHeatFluxBC](CoupledConvectiveHeatFluxBC.md) for a similar boundary condition coupled to variables.

!listing /convective_heat_flux/equilibrium.i block=BCs/right

!syntax parameters /BCs/ConvectiveHeatFluxBC

!syntax inputs /BCs/ConvectiveHeatFluxBC

!syntax children /BCs/ConvectiveHeatFluxBC
