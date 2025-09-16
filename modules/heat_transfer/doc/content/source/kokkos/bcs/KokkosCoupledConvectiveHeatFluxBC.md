# KokkosCoupledConvectiveHeatFluxBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledConvectiveHeatFluxBC](CoupledConvectiveHeatFluxBC.md). See the original document for details.

## Example Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_coupled_convective_heat_flux.i start=[right] end=[] include-end=true

!syntax parameters /KokkosBCs/KokkosCoupledConvectiveHeatFluxBC

!syntax inputs /KokkosBCs/KokkosCoupledConvectiveHeatFluxBC

!if-end!

!else
!include kokkos/kokkos_warning.md
