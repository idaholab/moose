# KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!if! function=hasCapability('kokkos')

This object provides Dirichlet boundary values for Kokkos linear finite volume advection-diffusion
kernels. The boundary value is supplied through a Kokkos-compatible `functor`. It is the Kokkos
analog of [LinearFVAdvectionDiffusionFunctorDirichletBC.md].

At present, this boundary condition is the only supported Dirichlet boundary treatment for
[KokkosLinearFVDiffusion.md].

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/diffusion/kokkos_diffusion-2d.i start=[dir] end=[] include-end=true

!syntax parameters /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!syntax inputs /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!syntax children /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md
