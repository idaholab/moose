# KokkosHomogenizedThermalConductivity

!if! function=hasCapability('kokkos')

This is the Kokkos version of [HomogenizedThermalConductivity](HomogenizedThermalConductivity.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_homogenization.i start=[k_xx] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosHomogenizedThermalConductivity

!syntax inputs /Postprocessors/KokkosHomogenizedThermalConductivity

!syntax children /Postprocessors/KokkosHomogenizedThermalConductivity

!if-end!

!else
!include kokkos/kokkos_warning.md
