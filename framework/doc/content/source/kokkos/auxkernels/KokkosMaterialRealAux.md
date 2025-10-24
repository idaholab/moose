# KokkosMaterialRealAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [MaterialRealAux](MaterialRealAux.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/materials/stateful_prop/kokkos_stateful_prop_test.i start=[prop1_output_init] end=[] include-end=true

!syntax parameters /KokkosAuxKernels/KokkosMaterialRealAux

!syntax inputs /KokkosAuxKernels/KokkosMaterialRealAux

!if-end!

!else
!include kokkos/kokkos_warning.md
