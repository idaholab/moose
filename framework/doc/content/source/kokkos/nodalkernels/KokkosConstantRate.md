# KokkosConstantRate

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ConstantRate](ConstantRate.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/nodalkernels/constant_rate/kokkos_constant_rate.i start=[constant_rate] end=[] include-end=true

!syntax parameters /KokkosNodalKernels/KokkosConstantRate

!syntax inputs /KokkosNodalKernels/KokkosConstantRate

!if-end!

!else
!include kokkos/kokkos_warning.md
