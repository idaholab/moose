# KokkosTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [TimeDerivative](TimeDerivative.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/kernels/coupled_time_derivative/kokkos_coupled_time_derivative_test.i start=[time_u] end=[] include-end=true

!syntax parameters /Kernels/KokkosTimeDerivative

!syntax inputs /Kernels/KokkosTimeDerivative

!syntax children /Kernels/KokkosTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md
