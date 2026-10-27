# KokkosLinearFVSource

!if! function=hasCapability('kokkos')

This is the Kokkos version of [LinearFVSource.md].

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/diffusion/kokkos_diffusion-2d.i start=[source] end=[] include-end=true

!syntax parameters /LinearFVKernels/KokkosLinearFVSource

!syntax inputs /LinearFVKernels/KokkosLinearFVSource

!syntax children /LinearFVKernels/KokkosLinearFVSource

!if-end!

!else
!include kokkos/kokkos_warning.md
