# KokkosElectricalConductivity

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElectricalConductivity](ElectricalConductivity.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_heat_source_materials.i start=[electrical_conductivity] end=[] include-end=true

!syntax parameters /Materials/KokkosElectricalConductivity

!syntax inputs /Materials/KokkosElectricalConductivity

!syntax children /Materials/KokkosElectricalConductivity

!if-end!

!else
!include kokkos/kokkos_warning.md
