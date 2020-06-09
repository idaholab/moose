# ADLevelSetBiMaterialRankTwo

!syntax description /Materials/ADLevelSetBiMaterialRankTwo

## Description

This material, `ADLevelSetBiMaterialRankTwo` determines the global material
property by switching the two Rank-2 tensor automatic differentiation material
properties with different base_name based on the level set values. It can be
used to select between the stresses computed by two ADComputeStress materials
that are defined for the level set positive domain and negative domain
respectively, based on the sign of the level set field. Those two
ADComputeStress materials can use completely different constitutive laws. See
[ADLevelSetBiMaterialReal](ADLevelSetBiMaterialReal.md) for more information.

## Example Input File Syntax

!listing modules/xfem/test/tests/bimaterials/glued_ad_bimaterials_2d.i block=Materials/combined_stress

!syntax parameters /Materials/ADLevelSetBiMaterialRankTwo

!syntax inputs /Materials/ADLevelSetBiMaterialRankTwo

!syntax children /Materials/ADLevelSetBiMaterialRankTwo
