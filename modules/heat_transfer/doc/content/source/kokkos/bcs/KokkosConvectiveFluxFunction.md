# KokkosConvectiveFluxFunction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ConvectiveFluxFunction](ConvectiveFluxFunction.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_convective_flux_function.i start=[right] end=[] include-end=true

!syntax parameters /BCs/KokkosConvectiveFluxFunction

!syntax inputs /BCs/KokkosConvectiveFluxFunction

!syntax children /BCs/KokkosConvectiveFluxFunction

!if-end!

!else
!include kokkos/kokkos_warning.md
