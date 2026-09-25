# KokkosComputeLinearElasticStress

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ComputeLinearElasticStress](ComputeLinearElasticStress.md). See the
original document for details. It consumes `elasticity_tensor` and `mechanical_strain`, and
declares `stress` and `Jacobian_mult`.

!alert note
The `elastic_strain` property of the non-Kokkos material is not declared.

The tangent `Jacobian_mult` equals the elasticity tensor, so it is stored once per subdomain rather
than once per quadrature point. For a rank-four tensor that is the dominant storage of the chain: on a
262144-element hexahedral mesh with eight quadrature points per element it is the difference between
648 bytes and 1.3 GB of device memory. The elasticity tensor this material reads must itself be stored
per subdomain, which is the default for
[KokkosComputeIsotropicElasticityTensor](KokkosComputeIsotropicElasticityTensor.md); an error is
reported if it is not.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/small_strain_elasticity/kokkos_small_strain.i block=Materials/stress

!syntax parameters /Materials/KokkosComputeLinearElasticStress

!syntax inputs /Materials/KokkosComputeLinearElasticStress

!syntax children /Materials/KokkosComputeLinearElasticStress

!if-end!

!else
!include kokkos/kokkos_warning.md
