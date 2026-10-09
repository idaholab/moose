# KokkosEquilibriumConstantAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [EquilibriumConstantAux](EquilibriumConstantAux.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_equilibrium_constant.i start=[logk] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosEquilibriumConstantAux

!syntax inputs /AuxKernels/KokkosEquilibriumConstantAux

!syntax children /AuxKernels/KokkosEquilibriumConstantAux

!if-end!

!else
!include kokkos/kokkos_warning.md
