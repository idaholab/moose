# LevelSetBiMaterialRankTwo

!syntax description /Materials/LevelSetBiMaterialRankTwo

## Description

This material, `LevelSetBiMaterialRankTwo` determines the global material property by switching the two Rank-2 tensor material properties with different base_name based on the level set values. It can be used to select between the stresses computed by two ComputeStress materials that are defined for the level set positive domain and negative domain respectively, based on the sign of the level set field. Those two ComputeStress materials can use completely different constitutive laws. See [LevelSetBiMaterialReal](LevelSetBiMaterialReal.md) for more information.

## Example Input File Syntax

!listing modules/xfem/test/tests/bimaterials/glued_bimaterials_2d.i block=Materials/combined_stress

!syntax parameters /Materials/LevelSetBiMaterialRankTwo

!syntax inputs /Materials/LevelSetBiMaterialRankTwo

!syntax children /Materials/LevelSetBiMaterialRankTwo

!bibtex bibliography
