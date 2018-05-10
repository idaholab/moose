# LevelSetBiMaterialReal

!syntax description /Materials/LevelSetBiMaterialReal

## Description

A level set function can represent the location of the interface between two material domains, which in general could have different material properties. A system has been developed to evaluate the proper models for a given material in such a system. This system operates by evaluating the material properties for both materials at every point, and then selecting the property appropriate for a given point.

This switching between material properties is done based on the `LevelSetBiMaterialBase` model, which selects between the material properties that are defined for the level set positive domain and negative domain respectively, based on the sign of the level set field. This material, `LevelSetBiMaterialReal`, is used to switch two Real material properties. The switching of Rank-2 and Rank-4 tensors material properties are implemented as [LevelSetBiMaterialRankTwo](LevelSetBiMaterialRankTwo.md) and [LevelSetBiMaterialRankFour](LevelSetBiMaterialRankFour.md), respectively.   

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/moving_diffusion.i block=Materials/diff_combined

!syntax parameters /Materials/LevelSetBiMaterialReal

!syntax inputs /Materials/LevelSetBiMaterialReal

!syntax children /Materials/LevelSetBiMaterialReal

!bibtex bibliography
