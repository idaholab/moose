# KokkosSideAverageValue

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SideAverageValue](SideAverageValue.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/side_integral/kokkos_side_average_value_test.i start=[average] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosSideAverageValue

!syntax inputs /Postprocessors/KokkosSideAverageValue

!syntax children /Postprocessors/KokkosSideAverageValue

!if-end!

!else
!include kokkos/kokkos_warning.md
