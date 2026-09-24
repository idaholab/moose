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

A second variant, `KokkosSymmetricVectorStressDivergence`, reads the stress and the Jacobian
multiplier in the minor-symmetric Mandel representation, `Real6` and `Real66`, instead of the dense
`Real33` and `Real3333`. The two are equivalent for a tangent with minor symmetry, which holds for
the small strain family, and the Mandel form contracts a 6x6 matrix with a 6-vector rather than an
81-entry tensor with a 9-entry one.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/small_strain_elasticity/kokkos_small_strain.i block=Kernels

!syntax parameters /Kernels/KokkosVectorStressDivergence

!syntax inputs /Kernels/KokkosVectorStressDivergence

!syntax children /Kernels/KokkosVectorStressDivergence

!if-end!

!else
!include kokkos/kokkos_warning.md
