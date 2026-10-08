# KokkosGrayLambertNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [GrayLambertNeumannBC](GrayLambertNeumannBC.md). See the original document for details.

!alert note
The surface radiation user object must be a Kokkos object derived from `KokkosGrayLambertSurfaceRadiationBase` such as [KokkosViewFactorObjectSurfaceRadiation](KokkosViewFactorObjectSurfaceRadiation.md). The Stefan-Boltzmann constant can be set by [!param](/BCs/KokkosGrayLambertNeumannBC/stefan_boltzmann_constant) to be consistent with the user object.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_gray_lambert_coupled.i start=[radiation] end=[] include-end=true

!syntax parameters /BCs/KokkosGrayLambertNeumannBC

!syntax inputs /BCs/KokkosGrayLambertNeumannBC

!syntax children /BCs/KokkosGrayLambertNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md
