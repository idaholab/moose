# KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!if! function=hasCapability('kokkos')

This object provides Dirichlet boundary values for Kokkos linear finite volume advection-diffusion
kernels. The boundary value is supplied through a Kokkos-compatible `functor`. It is the Kokkos
analog of [LinearFVAdvectionDiffusionFunctorDirichletBC.md].

Currently, the only accepted functor type is [KokkosParsedFunction.md], as we are waiting on
[relocatable device code (RDC)](KokkosFunctions/index.md#kokkos_rdc) support.

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/diffusion/kokkos_diffusion-2d.i start=[dir] end=[] include-end=true

!syntax parameters /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!syntax inputs /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!syntax children /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md
