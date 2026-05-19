# KokkosADDiffusion

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ADDiffusion](ADDiffusion.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/ad_diffusion/kokkos_2d_ad_diffusion_test.i start=[diff] end=[] include-end=true

!syntax parameters /Kernels/KokkosADDiffusion

!syntax inputs /Kernels/KokkosADDiffusion

!syntax children /Kernels/KokkosADDiffusion

!if-end!

!else
!include kokkos/kokkos_warning.md
