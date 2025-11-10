# KokkosTimeDerivativeNodalKernel

!if! function=hasCapability('kokkos')

This is the Kokkos version of [TimeDerivativeNodalKernel](TimeDerivativeNodalKernel.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/nodalkernels/constant_rate/kokkos_constant_rate.i start=[td] end=[] include-end=true

!syntax parameters /NodalKernels/KokkosTimeDerivativeNodalKernel

!syntax inputs /NodalKernels/KokkosTimeDerivativeNodalKernel

!if-end!

!else
!include kokkos/kokkos_warning.md
