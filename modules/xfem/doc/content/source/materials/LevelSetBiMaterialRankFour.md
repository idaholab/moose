# LevelSetBiMaterialRankFour

!syntax description /Materials/LevelSetBiMaterialRankFour

## Description

This material, `LevelSetBiMaterialRankFour` determines the global material property by switching the two Rank-4 tensor material properties with different base_name based on the level set values. It can be used to select between the Jacobian ($\frac{\partial\sigma}{\partial\epsilon}$) computed by two ComputeStress materials that are defined for the level set positive domain and negative domain respectively, based on the sign of the level set field. See [LevelSetBiMaterialReal](LevelSetBiMaterialReal.md) for more information.

## Example Input File Syntax

!listing modules/xfem/test/tests/bimaterials/glued_bimaterials_2d.i block=Materials/combined_dstressdstrain

!syntax parameters /Materials/LevelSetBiMaterialRankFour

!syntax inputs /Materials/LevelSetBiMaterialRankFour

!syntax children /Materials/LevelSetBiMaterialRankFour

!bibtex bibliography
