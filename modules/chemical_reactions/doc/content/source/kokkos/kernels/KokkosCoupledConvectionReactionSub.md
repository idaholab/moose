# KokkosCoupledConvectionReactionSub

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledConvectionReactionSub](CoupledConvectionReactionSub.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[a1conv] end=[] include-end=true

!syntax parameters /Kernels/KokkosCoupledConvectionReactionSub

!syntax inputs /Kernels/KokkosCoupledConvectionReactionSub

!syntax children /Kernels/KokkosCoupledConvectionReactionSub

!if-end!

!else
!include kokkos/kokkos_warning.md
