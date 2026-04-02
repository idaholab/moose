# KokkosLinearFVDiffusion

!if! function=hasCapability('kokkos')

This is the Kokkos version of [LinearFVDiffusion.md].

!alert note
`KokkosLinearFVDiffusion` does not support nonorthogonal correction yet

!alert note
`KokkosLinearFVDiffusion` currently only supports
[KokkosLinearFVAdvectionDiffusionFunctorDirichletBC.md] on external boundaries.

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/diffusion/kokkos_diffusion-2d.i start=[diffusion] end=[] include-end=true

!syntax parameters /LinearFVKernels/KokkosLinearFVDiffusion

!syntax inputs /LinearFVKernels/KokkosLinearFVDiffusion

!syntax children /LinearFVKernels/KokkosLinearFVDiffusion

!if-end!

!else
!include kokkos/kokkos_warning.md
