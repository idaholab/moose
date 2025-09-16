# KokkosNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [NeumannBC](NeumannBC.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/bcs/1d_neumann/kokkos_1d_neumann.i start=[right] end=[] include-end=true

!syntax parameters /KokkosBCs/KokkosNeumannBC

!syntax inputs /KokkosBCs/KokkosNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md
