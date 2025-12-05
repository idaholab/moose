# KokkosSideExtremeValue

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SideExtremeValue](SideExtremeValue.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/side_extreme_value/kokkos_nonlinear_variable.i start=[Postprocessors] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosSideExtremeValue

!syntax inputs /Postprocessors/KokkosSideExtremeValue

!syntax children /Postprocessors/KokkosSideExtremeValue

!if-end!

!else
!include kokkos/kokkos_warning.md
