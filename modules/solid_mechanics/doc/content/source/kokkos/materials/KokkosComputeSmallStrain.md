# KokkosComputeSmallStrain

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ComputeSmallStrain](ComputeSmallStrain.md). See the original document
for details.

[!param](/Materials/KokkosComputeSmallStrain/displacements) takes a single vector variable carrying
all of the displacement components, of a family such as `LAGRANGE_VEC`, rather than one scalar
variable per component.

!alert note
Eigenstrains, a global strain and the volumetric locking correction of the non-Kokkos material are
not available, so `mechanical_strain` always equals `total_strain`.

## Example Input Syntax

!listing solid_mechanics/test/tests/kokkos/small_strain_elasticity/kokkos_small_strain.i block=Materials/strain

!syntax parameters /Materials/KokkosComputeSmallStrain

!syntax inputs /Materials/KokkosComputeSmallStrain

!syntax children /Materials/KokkosComputeSmallStrain

!if-end!

!else
!include kokkos/kokkos_warning.md
