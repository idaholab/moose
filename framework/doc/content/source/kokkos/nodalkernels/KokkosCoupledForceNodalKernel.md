# KokkosCoupledForceNodalKernel

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledForceNodalKernel](CoupledForceNodalKernel.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/nodalkernels/constraint_enforcement/kokkos_lower_bound.i start=[forces] end=[] include-end=true

!syntax parameters /KokkosNodalKernels/KokkosCoupledForceNodalKernel

!syntax inputs /KokkosNodalKernels/KokkosCoupledForceNodalKernel

!if-end!

!else
!include kokkos/kokkos_warning.md
