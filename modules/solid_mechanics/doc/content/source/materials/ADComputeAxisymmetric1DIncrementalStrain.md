# ADComputeAxisymmetric1DIncrementalStrain

!syntax description /Materials/ADComputeAxisymmetric1DIncrementalStrain

## Description

`ADComputeAxisymmetric1DIncrementalStrain` computes the small strain increment
for a 1D axisymmetric plane-strain or generalized-plane-strain model using [!ac](AD).
It is the [!ac](AD) counterpart to [ComputeAxisymmetric1DIncrementalStrain.md] and supplies strains with all
derivatives required to form an exact Jacobian. Current displacement and
out-of-plane strain values are [!ac](AD) values; the old state values used to form the
strain increment remain regular values.

!template load file=modules/solid_mechanics/common/Axisymmetric1DStrainOverview.md.template

The out-of-plane strain can be supplied by either
[!param](/Materials/ADComputeAxisymmetric1DIncrementalStrain/scalar_out_of_plane_strain)
or [!param](/Materials/ADComputeAxisymmetric1DIncrementalStrain/out_of_plane_strain),
but not both. Scalar out-of-plane strain values are commonly used by
[Generalized Plane Strain](solid_mechanics/generalized_plane_strain.md)
models. If multiple scalar components are coupled,
[!param](/Materials/ADComputeAxisymmetric1DIncrementalStrain/subblock_index_provider)
selects which scalar component applies to the current element; without that
user object, component 0 is used.

## 1D Axisymmetric Strain Formulation

!template load file=modules/solid_mechanics/common/Axisymmetric1DIncrementalStrainFormulation.md.template

## Example Input File Syntax

Rather than directly specifying this model in the input file, it is recommended to
create it using the solid mechanics [QuasiStatic Physics](/Physics/SolidMechanics/QuasiStatic/index.md)
The usage examples listed below specify options within that block to create this model.

### Plane Strain

Under plane-strain assumptions, the strain in the axial direction
(typically the y-direction) is zero, which is appropriate for
modeling an axisymmetric body restrained against displacement at its ends.
This model can be created with plane-strain assumptions by setting 
`planar_formulation = PLANE_STRAIN` and `incremental = TRUE` in the
QuasiStatic Physics block.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_plane_strain_incremental.i block=Physics/SolidMechanics/QuasiStatic

Note that this example does not use [!ac](AD) but the relevant syntax is the
same for [!ac](AD).

### Generalized Plane Strain

Under generalized-plane-strain assumptions, the strain in the axial direction
(typically the y-direction) is nonzero, and calculated to satisfy equilibrium
in the axial direction. This is appropriate for modeling an axisymmetric body
with a long axial dimension, unrestrained at its ends.
This model can be created with generalized-plane-strain assumptions by setting 
`planar_formulation = GENERALIZED_PLANE_STRAIN` and `incremental = TRUE` in the
QuasiStatic Physics block.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_incremental.i block=Physics/SolidMechanics/QuasiStatic

The coupled scalar variable is defined in the same input file.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_incremental.i block=Variables/scalar_strain_yy

Note that this example does not use [!ac](AD) but the relevant syntax is the
same for [!ac](AD).

!syntax parameters /Materials/ADComputeAxisymmetric1DIncrementalStrain

!syntax inputs /Materials/ADComputeAxisymmetric1DIncrementalStrain

!syntax children /Materials/ADComputeAxisymmetric1DIncrementalStrain
