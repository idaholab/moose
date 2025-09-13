# KokkosConvectiveHeatFluxBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ConvectiveHeatFluxBC](ConvectiveHeatFluxBC.md). See the original document for details.

## Example Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_convective_heat_flux.i start=[right] end=[] include-end=true

!syntax parameters /KokkosBCs/KokkosConvectiveHeatFluxBC

!syntax inputs /KokkosBCs/KokkosConvectiveHeatFluxBC

!if-end!

!else
!include kokkos/kokkos_warning.md
