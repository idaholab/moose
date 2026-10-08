# KokkosDesorptionFromMatrix

!if! function=hasCapability('kokkos')

This is the Kokkos version of [DesorptionFromMatrix](DesorptionFromMatrix.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_langmuir_desorption.i start=[flow_from_matrix] end=[] include-end=true

!syntax parameters /Kernels/KokkosDesorptionFromMatrix

!syntax inputs /Kernels/KokkosDesorptionFromMatrix

!syntax children /Kernels/KokkosDesorptionFromMatrix

!if-end!

!else
!include kokkos/kokkos_warning.md
