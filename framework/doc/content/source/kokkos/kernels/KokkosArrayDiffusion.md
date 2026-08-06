# KokkosArrayDiffusion

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ArrayDiffusion](ArrayDiffusion.md). See the original document for details.

!alert note
[!param](Kernels/KokkosArrayDiffusion/diffusion_coefficient) may be a scalar, one-dimensional, or two-dimensional Kokkos material property. A one-dimensional property acts as a diagonal matrix, while a two-dimensional property couples array components.

!syntax parameters /Kernels/KokkosArrayDiffusion

!syntax inputs /Kernels/KokkosArrayDiffusion

!syntax children /Kernels/KokkosArrayDiffusion

!if-end!

!else
!include kokkos/kokkos_warning.md
