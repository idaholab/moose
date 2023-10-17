# ADConvectiveHeatFluxBC

!syntax description /BCs/ADConvectiveHeatFluxBC

This boundary condition computes convective heat flux $q'' = H \cdot (T - T_{inf})$, where $H$ is convective heat transfer coefficient,
$T$ is the temperature, and $T_{inf}$ is far field temperature.  Both $H$ and $T_{inf}$ are coupled as material properties.

!listing /ad_convective_heat_flux/equilibrium.i block=BCs/right

!syntax parameters /BCs/ADConvectiveHeatFluxBC

!syntax inputs /BCs/ADConvectiveHeatFluxBC

!syntax children /BCs/ADConvectiveHeatFluxBC
