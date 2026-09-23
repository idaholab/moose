# KokkosVectorStressDivergence

!if! function=hasCapability('kokkos')

Assembles the stress divergence term $(\sigma, \nabla \vec{\psi_i})$ for a single vector
displacement variable, taking the stress and its derivative with respect to the strain from the
`stress` and `Jacobian_mult` material properties. The constitutive model reaches the kernel
entirely through those two properties, so the kernel is independent of the material producing them.

Unlike [StressDivergenceTensors](StressDivergenceTensors.md), which acts on one scalar displacement
variable per component and so requires a `component` parameter, this kernel acts on one vector
variable of a family such as `LAGRANGE_VEC` and carries no `component` parameter. A single
kernel block therefore replaces the three of the non-Kokkos form.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/small_strain_elasticity/kokkos_small_strain.i block=Kernels

!syntax parameters /Kernels/KokkosVectorStressDivergence

!syntax inputs /Kernels/KokkosVectorStressDivergence

!syntax children /Kernels/KokkosVectorStressDivergence

!if-end!

!else
!include kokkos/kokkos_warning.md
