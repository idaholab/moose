# KokkosPostprocessorNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PostprocessorNeumannBC](PostprocessorNeumannBC.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/bcs/pp_neumann/kokkos_pp_neumann.i start=[right] end=[] include-end=true

!syntax parameters /KokkosBCs/KokkosPostprocessorNeumannBC

!syntax inputs /KokkosBCs/KokkosPostprocessorNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md
