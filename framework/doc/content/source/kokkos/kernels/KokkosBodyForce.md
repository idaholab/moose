# KokkosBodyForce

!if! function=hasCapability('kokkos')

This is the Kokkos version of [BodyForce](BodyForce.md). See the original document for details.

!alert note
The original version supports functions, but the Kokkos version does not support functions yet.

## Example Syntax

!listing test/tests/kokkos/kernels/2d_diffusion/kokkos_2d_diffusion_bodyforce_test.i start=[bf] end=[] include-end=true

!syntax parameters /KokkosKernels/KokkosBodyForce

!syntax inputs /KokkosKernels/KokkosBodyForce

!if-end!

!else
!include kokkos/kokkos_warning.md
