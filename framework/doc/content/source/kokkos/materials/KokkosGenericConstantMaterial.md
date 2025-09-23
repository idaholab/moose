# KokkosGenericConstantMaterial

!if! function=hasCapability('kokkos')

This is the Kokkos version of [GenericConstantMaterial](GenericConstantMaterial.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/material_coupled_force/kokkos_material_coupled_force.i start=[mat] end=[] include-end=true

!syntax parameters /KokkosMaterials/KokkosGenericConstantMaterial

!syntax inputs /KokkosMaterials/KokkosGenericConstantMaterial

!if-end!

!else
!include kokkos/kokkos_warning.md
