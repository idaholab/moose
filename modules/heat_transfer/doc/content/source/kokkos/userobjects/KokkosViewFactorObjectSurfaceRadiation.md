# KokkosViewFactorObjectSurfaceRadiation

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ViewFactorObjectSurfaceRadiation](ViewFactorObjectSurfaceRadiation.md). See the original document for details.

!alert note
The view factors are still provided by an original (non-Kokkos) view factor user object such as [SpecifiedViewFactor](SpecifiedViewFactor.md) or [UnobstructedPlanarViewFactor](UnobstructedPlanarViewFactor.md). The Kokkos user object accesses its results through the virtual Kokkos user object interface described in [Kokkos UserObjects System](syntax/KokkosUserObjects/index.md#virtual_user_objects), so the Kokkos objects consuming them only depend on the base class `KokkosGrayLambertSurfaceRadiationBase`.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_gray_lambert_coupled.i start=[cavity_radiation] end=[] include-end=true

!syntax parameters /UserObjects/KokkosViewFactorObjectSurfaceRadiation

!syntax inputs /UserObjects/KokkosViewFactorObjectSurfaceRadiation

!syntax children /UserObjects/KokkosViewFactorObjectSurfaceRadiation

!if-end!

!else
!include kokkos/kokkos_warning.md
