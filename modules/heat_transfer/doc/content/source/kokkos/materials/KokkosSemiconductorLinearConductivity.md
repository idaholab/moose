# KokkosSemiconductorLinearConductivity

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SemiconductorLinearConductivity](SemiconductorLinearConductivity.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_semiconductor_linear_conductivity.i start=[sigma] end=[] include-end=true

!syntax parameters /Materials/KokkosSemiconductorLinearConductivity

!syntax inputs /Materials/KokkosSemiconductorLinearConductivity

!syntax children /Materials/KokkosSemiconductorLinearConductivity

!if-end!

!else
!include kokkos/kokkos_warning.md
