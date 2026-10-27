# KokkosLinearFVFunctorNeumannBC

!if! function=hasCapability('kokkos')

This object provides prescribed outward normal gradient data for Kokkos linear finite volume
kernels. The outward normal gradient is supplied through a [KokkosParsedFunction.md] via the
[!param](/LinearFVBCs/KokkosLinearFVFunctorNeumannBC/functor) parameter.

The primitive relation supplied by this BC is

!equation
\nabla_n u = g

where $g$ is the prescribed outward normal gradient. Kernels apply their own physics coefficients
when consuming this relation; for example, a diffusion kernel multiplies by its diffusion
coefficient and face area. This BC therefore prescribes a normal gradient, not a total diffusive
flux.

For kernels that need a boundary value, this object also supplies the orthogonal extrapolation

!equation
u_b = u_C + g d_{Cf}

where $u_C$ is the cell value and $d_{Cf}$ is the cell-center-to-face-center distance.

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/diffusion/kokkos_diffusion-2d-neumann.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/KokkosLinearFVFunctorNeumannBC

!syntax inputs /LinearFVBCs/KokkosLinearFVFunctorNeumannBC

!syntax children /LinearFVBCs/KokkosLinearFVFunctorNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md
