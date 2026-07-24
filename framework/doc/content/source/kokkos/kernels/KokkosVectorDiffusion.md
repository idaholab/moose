# KokkosVectorDiffusion

!if! function=hasCapability('kokkos')

This is the Kokkos version of [VectorDiffusion](VectorDiffusion.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/transient_vector_diffusion/kokkos_transient_vector_diffusion.i start=[diff] end=[] include-end=true

!syntax parameters /Kernels/KokkosVectorDiffusion

!syntax inputs /Kernels/KokkosVectorDiffusion

!syntax children /Kernels/KokkosVectorDiffusion

!if-end!

!else
!include kokkos/kokkos_warning.md
