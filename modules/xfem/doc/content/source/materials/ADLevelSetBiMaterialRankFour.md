# ADLevelSetBiMaterialRankFour

!syntax description /Materials/ADLevelSetBiMaterialRankFour

## Description

This material, `ADLevelSetBiMaterialRankFour` determines the global material
property by switching the two AD Rank-4 tensor material properties with
different base_name based on the level set values.

While the non-AD equivalent,
[LevelSetBiMaterialRankFour](LevelSetBiMaterialRankFour.md), is used to compute
the Jacobian contribution ($\frac{\partial\sigma}{\partial\epsilon}$), this
value is not required in simulations using automatic differentiation. This class
can be used to determine other Rank-4 quantities, such as the elasticity tensor
(C$_{ijkl}$) for two materials that are defined for the level set positive
domain and negative domain respectively, based on the sign of the level set
field. See [ADLevelSetBiMaterialReal](ADLevelSetBiMaterialReal.md) for more
information.

## Example Input File Syntax

!listing modules/xfem/test/tests/bimaterials/glued_ad_bimaterials_2d.i block=Materials/combined_elasticity_tensor

!syntax parameters /Materials/ADLevelSetBiMaterialRankFour

!syntax inputs /Materials/ADLevelSetBiMaterialRankFour

!syntax children /Materials/ADLevelSetBiMaterialRankFour
