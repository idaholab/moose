# KokkosADNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ADNeumannBC](ADNeumannBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/kernels/ad_diffusion/kokkos_2d_ad_diffusion_neumannbc_test.i start=[right] end=[] include-end=true

!syntax parameters /BCs/KokkosADNeumannBC

!syntax inputs /BCs/KokkosADNeumannBC

!syntax children /BCs/KokkosADNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md
