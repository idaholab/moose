# ADComputeAxisymmetric1DIncrementalStrain

!syntax description /Materials/ADComputeAxisymmetric1DIncrementalStrain

## Description

`ADComputeAxisymmetric1DIncrementalStrain` computes the small strain increment
for a 1D axisymmetric generalized plane strain model using automatic
differentiation. It is the AD counterpart to
[ComputeAxisymmetric1DIncrementalStrain.md] and supplies strains with all
derivatives required to form an exact Jacobian. Current displacement and
out-of-plane strain values are AD values; the old state values used to form the
strain increment remain regular values.

!include modules/solid_mechanics/common/supplementalAxisymmetric1DStrainOverview.md

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

!include modules/solid_mechanics/common/supplementalAxisymmetric1DIncrementalStrainFormulation.md

## Example Input File Syntax

The following generalized plane strain test shows the corresponding non-AD
material block using a scalar out-of-plane strain. The same strain block
parameters apply to `ADComputeAxisymmetric1DIncrementalStrain`.

!listing modules/solid_mechanics/test/tests/1D_axisymmetric/axisymm_gps_incremental.i block=Materials/strain

!syntax parameters /Materials/ADComputeAxisymmetric1DIncrementalStrain

!syntax inputs /Materials/ADComputeAxisymmetric1DIncrementalStrain

!syntax children /Materials/ADComputeAxisymmetric1DIncrementalStrain
