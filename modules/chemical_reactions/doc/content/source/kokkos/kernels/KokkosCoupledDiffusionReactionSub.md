# KokkosCoupledDiffusionReactionSub

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledDiffusionReactionSub](CoupledDiffusionReactionSub.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[a1diff] end=[] include-end=true

!syntax parameters /Kernels/KokkosCoupledDiffusionReactionSub

!syntax inputs /Kernels/KokkosCoupledDiffusionReactionSub

!syntax children /Kernels/KokkosCoupledDiffusionReactionSub

!if-end!

!else
!include kokkos/kokkos_warning.md
