# KokkosCoupledTimeDerivative

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledTimeDerivative](CoupledTimeDerivative.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/kernels/coupled_time_derivative/kokkos_coupled_time_derivative_test.i start=[time_v] end=[] include-end=true

!syntax parameters /Kernels/KokkosCoupledTimeDerivative

!syntax inputs /Kernels/KokkosCoupledTimeDerivative

!if-end!

!else
!include kokkos/kokkos_warning.md
