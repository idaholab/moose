# KokkosAqueousEquilibriumRxnAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [AqueousEquilibriumRxnAux](AqueousEquilibriumRxnAux.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[pa2eq] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosAqueousEquilibriumRxnAux

!syntax inputs /AuxKernels/KokkosAqueousEquilibriumRxnAux

!syntax children /AuxKernels/KokkosAqueousEquilibriumRxnAux

!if-end!

!else
!include kokkos/kokkos_warning.md
