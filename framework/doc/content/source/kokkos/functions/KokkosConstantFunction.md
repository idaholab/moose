# KokkosConstantFunction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ConstantFunction](ConstantFunction.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/functions/constant_function/kokkos_constant_function.i start=[constant] end=[] include-end=true

!syntax parameters /KokkosFunctions/KokkosConstantFunction

!syntax inputs /KokkosFunctions/KokkosConstantFunction

!if-end!

!else
!include kokkos/kokkos_warning.md
