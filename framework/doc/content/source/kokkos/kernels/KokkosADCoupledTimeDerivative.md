# KokkosADCoupledTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ADCoupledTimeDerivative](ADCoupledTimeDerivative.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/ad_time_derivative/kokkos_ad_coupled_time_derivative_test.i start=[time_v] end=[] include-end=true

!syntax parameters /Kernels/KokkosADCoupledTimeDerivative

!syntax inputs /Kernels/KokkosADCoupledTimeDerivative

!syntax children /Kernels/KokkosADCoupledTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md
