# ADLevelSetBiMaterialReal

!syntax description /Materials/ADLevelSetBiMaterialReal

## Description

A level set function can represent the location of the interface between two
material domains, which in general could have different material properties. A
system has been developed to evaluate the proper models for a given material in
such a system. This system operates by evaluating the material properties for
both materials at every point, and then selecting the property appropriate for a
given point.

This switching between automatic differentiation material properties is done
based on the `ADLevelSetBiMaterialBase` model, which selects between the AD
material properties that are defined for the level set positive domain and
negative domain respectively, based on the sign of the level set field. This
material, `ADLevelSetBiMaterialReal`, is used to switch two Real material
properties. The switching of Rank-2 and Rank-4 tensor automatic differentiation
material properties are implemented as
[ADLevelSetBiMaterialRankTwo](ADLevelSetBiMaterialRankTwo.md) and
[ADLevelSetBiMaterialRankFour](ADLevelSetBiMaterialRankFour.md), respectively.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/moving_ad_diffusion.i block=Materials/diff_combined

!syntax parameters /Materials/ADLevelSetBiMaterialReal

!syntax inputs /Materials/ADLevelSetBiMaterialReal

!syntax children /Materials/ADLevelSetBiMaterialReal
