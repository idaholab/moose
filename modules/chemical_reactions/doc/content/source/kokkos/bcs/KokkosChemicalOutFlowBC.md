# KokkosChemicalOutFlowBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ChemicalOutFlowBC](ChemicalOutFlowBC.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_aqueous_equilibrium.i start=[a_right] end=[] include-end=true

!syntax parameters /BCs/KokkosChemicalOutFlowBC

!syntax inputs /BCs/KokkosChemicalOutFlowBC

!syntax children /BCs/KokkosChemicalOutFlowBC

!if-end!

!else
!include kokkos/kokkos_warning.md
