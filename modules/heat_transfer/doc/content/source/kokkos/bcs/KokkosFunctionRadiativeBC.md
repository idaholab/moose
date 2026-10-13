# KokkosFunctionRadiativeBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [FunctionRadiativeBC](FunctionRadiativeBC.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_function_radiative_bc.i start=[bot_right] end=[] include-end=true

!syntax parameters /BCs/KokkosFunctionRadiativeBC

!syntax inputs /BCs/KokkosFunctionRadiativeBC

!syntax children /BCs/KokkosFunctionRadiativeBC

!if-end!

!else
!include kokkos/kokkos_warning.md
