# KokkosUpperBoundNodalKernel

!if! function=hasCapability('kokkos')

This is the Kokkos version of [UpperBoundNodalKernel](UpperBoundNodalKernel.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/nodalkernels/constraint_enforcement/kokkos_upper_bound.i start=[positive_constraint] end=[] include-end=true

!syntax parameters /KokkosNodalKernels/KokkosUpperBoundNodalKernel

!syntax inputs /KokkosNodalKernels/KokkosUpperBoundNodalKernel

!if-end!

!else
!include kokkos/kokkos_warning.md
