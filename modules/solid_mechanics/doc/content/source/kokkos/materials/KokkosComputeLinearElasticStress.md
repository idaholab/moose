# KokkosComputeLinearElasticStress

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ComputeLinearElasticStress](ComputeLinearElasticStress.md). See the
original document for details. It consumes `elasticity_tensor` and `mechanical_strain`, and
declares `stress` and `Jacobian_mult`.

!alert note
The `elastic_strain` property of the non-Kokkos material is not declared.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/small_strain_elasticity/kokkos_small_strain.i block=Materials/stress

!syntax parameters /Materials/KokkosComputeLinearElasticStress

!syntax inputs /Materials/KokkosComputeLinearElasticStress

!syntax children /Materials/KokkosComputeLinearElasticStress

!if-end!

!else
!include kokkos/kokkos_warning.md
