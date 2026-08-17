# ADComputeAxisymmetric1DSmallStrain

!syntax description /Materials/ADComputeAxisymmetric1DSmallStrain

## Description

`ADComputeAxisymmetric1DSmallStrain` computes the small total strain for a 1D
axisymmetric generalized plane strain model using [!ac](AD).
It is the [!ac](AD) counterpart to [ComputeAxisymmetric1DSmallStrain.md] and supplies
strains with all derivatives required to form an exact Jacobian.

!template load file=modules/solid_mechanics/common/Axisymmetric1DStrainOverview.md.template

The out-of-plane strain can be supplied by either
[!param](/Materials/ADComputeAxisymmetric1DSmallStrain/scalar_out_of_plane_strain)
or [!param](/Materials/ADComputeAxisymmetric1DSmallStrain/out_of_plane_strain),
but not both. Scalar out-of-plane strain values are commonly used by
[Generalized Plane Strain](solid_mechanics/generalized_plane_strain.md)
models. If multiple scalar components are coupled,
[!param](/Materials/ADComputeAxisymmetric1DSmallStrain/subblock_index_provider)
selects which scalar component applies to the current element; without that
user object, component 0 is used.

## 1D Axisymmetric Strain Formulation

!template load file=modules/solid_mechanics/common/Axisymmetric1DSmallStrainFormulation.md.template

## Example Input File Syntax

Rather than directly specifying this model in the input file, it is recommended to
create it using the solid mechanics [QuasiStatic Physics](/Physics/SolidMechanics/QuasiStatic/index.md)
The usage examples listed below specify options within that block to create this model.

### Plane Strain

Under plane-strain assumptions, the strain in the axial direction
(typically the y-direction) is zero, which is appropriate for
modeling an axisymmetric body restrained against displacement at its ends.
This model can be created with plane-strain assumptions by setting 
`planar_formulation = PLANE_STRAIN`, `incremental = FALSE`, and `strain = SMALL` in the
QuasiStatic Physics block.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_plane_strain_small.i block=Physics/SolidMechanics/QuasiStatic

Note that this example does not use [!ac](AD) but the relevant syntax is the
same for [!ac](AD).

### Generalized Plane Strain

Under generalized-plane-strain assumptions, the strain in the axial direction
(typically the y-direction) is nonzero, and calculated to satisfy equilibrium
in the axial direction. This is appropriate for modeling an axisymmetric body
with a long axial dimension, unrestrained at its ends.
This model can be created with generalized-plane-strain assumptions by setting 
`planar_formulation = GENERALIZED_PLANE_STRAIN`, `incremental = FALSE`, and `strain = SMALL` in the
QuasiStatic Physics block.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_small.i block=Physics/SolidMechanics/QuasiStatic

The coupled scalar variable is defined in the same input file.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_small.i block=Variables/scalar_strain_yy

Note that this example does not use [!ac](AD) but the relevant syntax is the
same for [!ac](AD).

!syntax parameters /Materials/ADComputeAxisymmetric1DSmallStrain

!syntax inputs /Materials/ADComputeAxisymmetric1DSmallStrain

!syntax children /Materials/ADComputeAxisymmetric1DSmallStrain
