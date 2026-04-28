# KokkosParsedAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ParsedAux](ParsedAux.md). See the original document for details.

!alert note
The Kokkos version does not support functors yet. Instead, postprocessors, variables, material properties, and functions can be specified for the use in the parsed expression through separate parameters.

## Example Input Syntax

!listing test/tests/kokkos/auxkernels/parsed/kokkos_parsed_aux_test.i start=[set_parsed] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosParsedAux

!syntax inputs /AuxKernels/KokkosParsedAux

!syntax children /AuxKernels/KokkosParsedAux

!if-end!

!else
!include kokkos/kokkos_warning.md
