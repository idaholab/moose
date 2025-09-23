# KokkosDiffusion

!if! function=hasCapability('kokkos')

This is the Kokkos version of [Diffusion](Diffusion.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/kernels/2d_diffusion/kokkos_2d_diffusion_test.i start=[diff] end=[] include-end=true

!syntax parameters /KokkosKernels/KokkosDiffusion

!syntax inputs /KokkosKernels/KokkosDiffusion

!if-end!

!else
!include kokkos/kokkos_warning.md
