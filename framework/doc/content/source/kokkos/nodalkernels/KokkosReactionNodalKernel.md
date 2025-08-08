# KokkosReactionNodalKernel

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ReactionNodalKernel.md]. See the original document for details.

!syntax parameters /KokkosNodalKernels/KokkosReactionNodalKernel

!syntax inputs /KokkosNodalKernels/KokkosReactionNodalKernel

!if-end!

!else
!include kokkos/kokkos_warning.md
