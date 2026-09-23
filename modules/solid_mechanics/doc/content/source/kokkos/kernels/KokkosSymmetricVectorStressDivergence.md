# KokkosSymmetricVectorStressDivergence

!if! function=hasCapability('kokkos')

The minor-symmetric form of [KokkosVectorStressDivergence](KokkosVectorStressDivergence.md),
consuming `stress` as a `Real6` and `Jacobian_mult` as a `Real66` rather than as dense tensors.

Both objects assemble the same operator. Storing the tangent in Mandel notation makes the double
contraction a six by six matrix-vector product, at 36 multiplies against the 81 of the dense form,
and it is the representation NEML2 produces, so a NEML2-backed chain reaches this kernel without a
change of representation.

Use the dense [KokkosVectorStressDivergence](KokkosVectorStressDivergence.md) where the tangent has
no minor symmetry, as the derivative of the first Piola-Kirchhoff stress with respect to the
deformation gradient does not.

!syntax parameters /Kernels/KokkosSymmetricVectorStressDivergence

!syntax inputs /Kernels/KokkosSymmetricVectorStressDivergence

!syntax children /Kernels/KokkosSymmetricVectorStressDivergence

!if-end!

!else
!include kokkos/kokkos_warning.md
