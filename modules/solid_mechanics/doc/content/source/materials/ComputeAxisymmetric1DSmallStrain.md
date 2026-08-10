# ComputeAxisymmetric1DSmallStrain

!syntax description /Materials/ComputeAxisymmetric1DSmallStrain

## Description

`ComputeAxisymmetric1DSmallStrain` computes the small total strain for a 1D
axisymmetric generalized plane strain model. When the solid mechanics action is
used with `use_automatic_differentiation = true`, the action instead selects
the AD counterpart, [ADComputeAxisymmetric1DSmallStrain.md].

!template load file=modules/solid_mechanics/common/Axisymmetric1DStrainOverview.md.template

The out-of-plane strain can be supplied by either
[!param](/Materials/ComputeAxisymmetric1DSmallStrain/scalar_out_of_plane_strain)
or [!param](/Materials/ComputeAxisymmetric1DSmallStrain/out_of_plane_strain),
but not both. Scalar out-of-plane strain values are commonly used by
[Generalized Plane Strain](solid_mechanics/generalized_plane_strain.md)
models. If multiple scalar components are coupled,
[!param](/Materials/ComputeAxisymmetric1DSmallStrain/subblock_index_provider)
selects which scalar component applies to the current element; without that
user object, component 0 is used.

## 1D Axisymmetric Strain Formulation

!template load file=modules/solid_mechanics/common/Axisymmetric1DSmallStrainFormulation.md.template

## Example Input File Syntax

The following generalized plane strain test uses the scalar out-of-plane strain
option with `ComputeAxisymmetric1DSmallStrain`.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_small.i block=Materials/strain

The coupled scalar variable is defined in the same input file.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_small.i block=Variables/scalar_strain_yy

!syntax parameters /Materials/ComputeAxisymmetric1DSmallStrain

!syntax inputs /Materials/ComputeAxisymmetric1DSmallStrain

!syntax children /Materials/ComputeAxisymmetric1DSmallStrain
