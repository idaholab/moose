# KokkosTotalConcentrationAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [TotalConcentrationAux](TotalConcentrationAux.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_reaction_network_co2_h2o.i start=[total_h+] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosTotalConcentrationAux

!syntax inputs /AuxKernels/KokkosTotalConcentrationAux

!syntax children /AuxKernels/KokkosTotalConcentrationAux

!if-end!

!else
!include kokkos/kokkos_warning.md
