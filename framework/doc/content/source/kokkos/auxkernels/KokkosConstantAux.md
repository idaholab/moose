# KokkosConstantAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ConstantAux](ConstantAux.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/bcs/coupled_var_neumann/kokkos_on_off.i start=[active_right] end=[] include-end=true

!syntax parameters /KokkosAuxKernels/KokkosConstantAux

!syntax inputs /KokkosAuxKernels/KokkosConstantAux

!if-end!

!else
!include kokkos/kokkos_warning.md
