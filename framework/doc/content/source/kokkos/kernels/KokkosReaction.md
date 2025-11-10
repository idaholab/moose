# KokkosReaction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [Reaction](Reaction.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/kernels/material_coupled_force/kokkos_material_coupled_force.i start=[reaction] end=[] include-end=true

!syntax parameters /Kernels/KokkosReaction

!syntax inputs /Kernels/KokkosReaction

!if-end!

!else
!include kokkos/kokkos_warning.md
