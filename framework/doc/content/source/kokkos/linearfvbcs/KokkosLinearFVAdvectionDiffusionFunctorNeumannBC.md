# KokkosLinearFVAdvectionDiffusionFunctorNeumannBC

!if! function=hasCapability('kokkos')

This object applies a prescribed outward normal flux on a boundary for Kokkos linear finite volume
advection-diffusion kernels. It is the Kokkos analog of
[LinearFVAdvectionDiffusionFunctorNeumannBC.md].

The outward normal flux $\phi_b$ at the boundary is supplied directly through a
[KokkosParsedFunction.md] via the
[!param](/LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorNeumannBC/functor) parameter, with the
sign convention that positive values represent outflow. The contribution to the right-hand side for
a boundary face is

!equation
\phi_b \, A_b

where $A_b$ is the face area. This BC contributes nothing to the system matrix.

!alert note
This boundary condition should only be used for problems that involve diffusion or
advection-diffusion.

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/diffusion/kokkos_diffusion-2d-neumann.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorNeumannBC

!syntax inputs /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorNeumannBC

!syntax children /LinearFVBCs/KokkosLinearFVAdvectionDiffusionFunctorNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md
