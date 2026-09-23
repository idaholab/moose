# KokkosComputeIsotropicElasticityTensor

!if! function=hasCapability('kokkos')

This is the Kokkos version of
[ComputeIsotropicElasticityTensor](ComputeIsotropicElasticityTensor.md). See the original document
for details. Exactly two of [!param](/Materials/KokkosComputeIsotropicElasticityTensor/bulk_modulus),
[!param](/Materials/KokkosComputeIsotropicElasticityTensor/lambda),
[!param](/Materials/KokkosComputeIsotropicElasticityTensor/poissons_ratio),
[!param](/Materials/KokkosComputeIsotropicElasticityTensor/shear_modulus) and
[!param](/Materials/KokkosComputeIsotropicElasticityTensor/youngs_modulus) must be supplied.

The elasticity tensor varies neither in space nor in time, so this material stores one tensor per
subdomain rather than one per quadrature point.

!alert note
The `elasticity_tensor_prefactor` function and the `effective_stiffness` property of the
non-Kokkos material are not available.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/small_strain_elasticity/kokkos_small_strain.i block=Materials/elasticity_tensor

!syntax parameters /Materials/KokkosComputeIsotropicElasticityTensor

!syntax inputs /Materials/KokkosComputeIsotropicElasticityTensor

!syntax children /Materials/KokkosComputeIsotropicElasticityTensor

!if-end!

!else
!include kokkos/kokkos_warning.md
