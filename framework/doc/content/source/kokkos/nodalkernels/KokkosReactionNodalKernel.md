# KokkosReactionNodalKernel

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ReactionNodalKernel](ReactionNodalKernel.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/nodalkernels/reaction/kokkos_reaction.i start=[reaction] end=[] include-end=true

!syntax parameters /KokkosNodalKernels/KokkosReactionNodalKernel

!syntax inputs /KokkosNodalKernels/KokkosReactionNodalKernel

!if-end!

!else
!include kokkos/kokkos_warning.md
