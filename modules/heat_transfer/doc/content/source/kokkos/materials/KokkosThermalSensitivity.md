# KokkosThermalSensitivity

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ThermalSensitivity](ThermalSensitivity.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_temperature_dependent.i start=[sensitivity] end=[] include-end=true

!syntax parameters /Materials/KokkosThermalSensitivity

!syntax inputs /Materials/KokkosThermalSensitivity

!syntax children /Materials/KokkosThermalSensitivity

!if-end!

!else
!include kokkos/kokkos_warning.md
