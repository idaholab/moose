# KokkosLinearFVFunctorDirichletBC

!if! function=hasCapability('kokkos')

This object provides Dirichlet boundary value data for Kokkos linear finite volume kernels. The
boundary value is supplied through a Kokkos-compatible `functor`.

For kernels that need boundary normal gradient data, this object supplies the affine relation

!equation
\nabla_n u = -\frac{1}{d_{Cf}} u_C + \frac{g}{d_{Cf}}

where $u_C$ is the cell value, $g$ is the prescribed boundary value, and $d_{Cf}$ is the
cell-center-to-face-center distance. Kernels apply their own physics coefficients when consuming
this relation.

Currently, the only accepted functor type is [KokkosParsedFunction.md], as we are waiting on
[relocatable device code (RDC)](KokkosFunctions/index.md#kokkos_rdc) support.

## Example Syntax

!listing test/tests/kokkos/linearfvkernels/diffusion/kokkos_diffusion-2d.i start=[dir] end=[] include-end=true

!syntax parameters /LinearFVBCs/KokkosLinearFVFunctorDirichletBC

!syntax inputs /LinearFVBCs/KokkosLinearFVFunctorDirichletBC

!syntax children /LinearFVBCs/KokkosLinearFVFunctorDirichletBC

!if-end!

!else
!include kokkos/kokkos_warning.md
