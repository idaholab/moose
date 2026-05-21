# KokkosADDirichletBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ADDirichletBC](ADDirichletBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/ad_diffusion/kokkos_2d_ad_diffusion_test.i start=[left] end=[] include-end=true

!syntax parameters /BCs/KokkosADDirichletBC

!syntax inputs /BCs/KokkosADDirichletBC

!syntax children /BCs/KokkosADDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md
