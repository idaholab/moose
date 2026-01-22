# KokkosElementAverageMaterialProperty

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ElementAverageMaterialProperty](ElementAverageMaterialProperty.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/postprocessors/element_integral_material_property/kokkos_element_average_material_property.i start=[prop_average] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosElementAverageMaterialProperty

!syntax inputs /Postprocessors/KokkosElementAverageMaterialProperty

!syntax children /Postprocessors/KokkosElementAverageMaterialProperty

!if-end!

!else
!include kokkos/kokkos_warning.md
