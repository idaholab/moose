# KokkosGrayLambertSurfaceRadiationPP

!if! function=hasCapability('kokkos')

This is the Kokkos version of [GrayLambertSurfaceRadiationPP](GrayLambertSurfaceRadiationPP.md). See the original document for details.

!alert note
The surface radiation user object must be a Kokkos object derived from `KokkosGrayLambertSurfaceRadiationBase` such as [KokkosViewFactorObjectSurfaceRadiation](KokkosViewFactorObjectSurfaceRadiation.md).

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_gray_lambert_coupled.i start=[qdot_left] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosGrayLambertSurfaceRadiationPP

!syntax inputs /Postprocessors/KokkosGrayLambertSurfaceRadiationPP

!syntax children /Postprocessors/KokkosGrayLambertSurfaceRadiationPP

!if-end!

!else
!include kokkos/kokkos_warning.md
