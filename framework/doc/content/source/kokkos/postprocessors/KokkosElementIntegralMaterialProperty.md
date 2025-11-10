# KokkosElementIntegralMaterialProperty

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementIntegralMaterialProperty](ElementIntegralMaterialProperty.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/element_integral_material_property/kokkos_element_integral_material_property.i start=[prop_integral] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosElementIntegralMaterialProperty

!syntax inputs /Postprocessors/KokkosElementIntegralMaterialProperty

!syntax children /Postprocessors/KokkosElementIntegralMaterialProperty

!if-end!

!else
!include kokkos/kokkos_warning.md
