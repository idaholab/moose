# KokkosSideIntegralMaterialProperty

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SideIntegralMaterialProperty](SideIntegralMaterialProperty.md). See the original document for details.

!alert note
`KokkosSideIntegralMaterialProperty` only supports scalar material properties.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/side_integral/kokkos_side_integral_material_property.i start=[integral] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosSideIntegralMaterialProperty

!syntax inputs /Postprocessors/KokkosSideIntegralMaterialProperty

!syntax children /Postprocessors/KokkosSideIntegralMaterialProperty

!if-end!

!else
!include kokkos/kokkos_warning.md
