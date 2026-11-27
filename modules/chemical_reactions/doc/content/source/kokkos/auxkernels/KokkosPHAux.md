# KokkosPHAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PHAux](PHAux.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_reaction_network_water_dissociation.i start=[ph] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosPHAux

!syntax inputs /AuxKernels/KokkosPHAux

!syntax children /AuxKernels/KokkosPHAux

!if-end!

!else
!include kokkos/kokkos_warning.md
