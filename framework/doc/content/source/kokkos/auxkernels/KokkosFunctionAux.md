# KokkosFunctionAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [FunctionAux](FunctionAux.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/functions/piecewise_constant/kokkos_piecewise_constant.i block=KokkosAuxKernels

!syntax parameters /KokkosAuxKernels/KokkosFunctionAux

!syntax inputs /KokkosAuxKernels/KokkosFunctionAux

!if-end!

!else
!include kokkos/kokkos_warning.md
