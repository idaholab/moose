# KokkosThermalConductivity

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ThermalConductivity](ThermalConductivity.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_temperature_dependent.i start=[k_eff] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosThermalConductivity

!syntax inputs /Postprocessors/KokkosThermalConductivity

!syntax children /Postprocessors/KokkosThermalConductivity

!if-end!

!else
!include kokkos/kokkos_warning.md
