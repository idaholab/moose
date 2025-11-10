# KokkosBodyForce

!if! function=hasCapability('kokkos')

This is the Kokkos version of [BodyForce](BodyForce.md). See the original document for details.

!alert note
Kokkos-MOOSE does not fully support functions yet. Therefore, [!param](/Kernels/BodyForce/function) is not available.

## Example Syntax

!listing test/tests/kokkos/kernels/2d_diffusion/kokkos_2d_diffusion_bodyforce_test.i start=[bf] end=[] include-end=true

!syntax parameters /Kernels/KokkosBodyForce

!syntax inputs /Kernels/KokkosBodyForce

!syntax children /Kernels/KokkosBodyForce

!if-end!

!else
!include kokkos/kokkos_warning.md
