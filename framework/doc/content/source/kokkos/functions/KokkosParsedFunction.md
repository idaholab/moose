# KokkosParsedFunction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ParsedFunction](MooseParsedFunction.md). See the original document for details.

!alert note
The Kokkos version does not support using scalar variables or other functions for [!param](/Functions/KokkosParsedFunction/symbol_values) yet.

## Example Syntax

!listing test/tests/kokkos/functions/parsed_function/kokkos_parsed_function.i block=Functions

!syntax parameters /Functions/KokkosParsedFunction

!syntax inputs /Functions/KokkosParsedFunction

!syntax children /Functions/KokkosParsedFunction

!if-end!

!else
!include kokkos/kokkos_warning.md
