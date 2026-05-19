# KokkosADTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ADTimeDerivative](ADTimeDerivative.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/ad_time_derivative/kokkos_ad_coupled_time_derivative_test.i start=[time_u] end=[] include-end=true

!syntax parameters /Kernels/KokkosADTimeDerivative

!syntax inputs /Kernels/KokkosADTimeDerivative

!syntax children /Kernels/KokkosADTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md
