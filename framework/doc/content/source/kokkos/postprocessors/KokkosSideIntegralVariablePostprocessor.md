# KokkosSideIntegralVariablePostprocessor

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SideIntegralVariablePostprocessor](SideIntegralVariablePostprocessor.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/side_integral/kokkos_side_integral_value_test.i start=[integral] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosSideIntegralVariablePostprocessor

!syntax inputs /Postprocessors/KokkosSideIntegralVariablePostprocessor

!syntax children /Postprocessors/KokkosSideIntegralVariablePostprocessor

!if-end!

!else
!include kokkos/kokkos_warning.md
