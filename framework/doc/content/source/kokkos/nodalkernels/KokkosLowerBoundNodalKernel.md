# KokkosLowerBoundNodalKernel

!if! function=hasCapability('kokkos')

This is the Kokkos version of [LowerBoundNodalKernel](LowerBoundNodalKernel.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/nodalkernels/constraint_enforcement/kokkos_lower_bound.i start=[positive_constraint] end=[] include-end=true

!syntax parameters /NodalKernels/KokkosLowerBoundNodalKernel

!syntax inputs /NodalKernels/KokkosLowerBoundNodalKernel

!if-end!

!else
!include kokkos/kokkos_warning.md
