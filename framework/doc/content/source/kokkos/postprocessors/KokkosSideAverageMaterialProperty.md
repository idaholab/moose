# KokkosSideAverageMaterialProperty

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SideAverageMaterialProperty](SideAverageMaterialProperty.md). See the original document for details.

!alert note
`KokkosSideAverageMaterialProperty` only supports scalar material properties.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/side_integral/kokkos_side_average_value_test.i start=[average] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosSideAverageMaterialProperty

!syntax inputs /Postprocessors/KokkosSideAverageMaterialProperty

!syntax children /Postprocessors/KokkosSideAverageMaterialProperty

!if-end!

!else
!include kokkos/kokkos_warning.md
