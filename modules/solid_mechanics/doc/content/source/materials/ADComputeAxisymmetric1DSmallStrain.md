# ADComputeAxisymmetric1DSmallStrain

!syntax description /Materials/ADComputeAxisymmetric1DSmallStrain

## Description

`ADComputeAxisymmetric1DSmallStrain` computes the small total strain for a 1D
axisymmetric generalized plane strain model using automatic differentiation.
It is the AD counterpart to [ComputeAxisymmetric1DSmallStrain.md] and supplies
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

The following generalized plane strain input uses the QuasiStatic Physics with
automatic differentiation; the action selects `ADComputeAxisymmetric1DSmallStrain`
as the strain calculator.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymmetric_gps_ad_jacobian.i block=Physics/SolidMechanics/QuasiStatic

!syntax parameters /Materials/ADComputeAxisymmetric1DSmallStrain

!syntax inputs /Materials/ADComputeAxisymmetric1DSmallStrain

!syntax children /Materials/ADComputeAxisymmetric1DSmallStrain
