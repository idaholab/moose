# KokkosDesorptionToPorespace

!if! function=hasCapability('kokkos')

This is the Kokkos version of [DesorptionToPorespace](DesorptionToPorespace.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_langmuir_desorption.i start=[flux_to_porespace] end=[] include-end=true

!syntax parameters /Kernels/KokkosDesorptionToPorespace

!syntax inputs /Kernels/KokkosDesorptionToPorespace

!syntax children /Kernels/KokkosDesorptionToPorespace

!if-end!

!else
!include kokkos/kokkos_warning.md
