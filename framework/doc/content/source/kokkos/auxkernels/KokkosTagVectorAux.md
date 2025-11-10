# KokkosTagVectorAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [TagVectorAux](TagVectorAux.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/tag/kokkos_2d_diffusion_tag_vector.i start=[TagVectorAux1] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosTagVectorAux

!syntax inputs /AuxKernels/KokkosTagVectorAux

!if-end!

!else
!include kokkos/kokkos_warning.md
