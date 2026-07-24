# KokkosLinearFVAdvection

!if! function=hasCapability('kokkos')

This is the Kokkos version of [LinearFVAdvection.md] for first-order upwind interpolation.

!alert note
`KokkosLinearFVAdvection` currently supports constant velocities and first-order upwind
interpolation.

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/advection-diffusion/kokkos_advection_diffusion-1d.i start=[advection] end=[] include-end=true

!syntax parameters /LinearFVKernels/KokkosLinearFVAdvection

!syntax inputs /LinearFVKernels/KokkosLinearFVAdvection

!syntax children /LinearFVKernels/KokkosLinearFVAdvection

!if-end!

!else
!include kokkos/kokkos_warning.md
