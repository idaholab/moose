# XFEMCutSwitchingMaterial

!syntax description /Materials/XFEMCutSwitchingMaterialReal

To allow using XFEM to model interfaces between materials, the XFEM system uses cut subdomain IDs to denote the subset of a standard MOOSE subdomain (element block) that a material point belongs to. Based on these cut subdomains, `XFEMCutSwitchingMaterial` switches between material properties depending on which cut subdomain a point is located within. This class reads in multiple versions of a material property, all with different values of `base_name`, and sets the value of the material property without that base name to the name of the property that applies to the current cut subdomain.

This class has four instantiations described below that are used to switch between properties with specific types:

## XFEMCutSwitchingMaterialReal

`XFEMCutSwitchingMaterialReal` switches between two material properties of type `Real` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/XFEMCutSwitchingMaterialReal

!syntax inputs /Materials/XFEMCutSwitchingMaterialReal

## XFEMCutSwitchingMaterialRankTwoTensor

`XFEMCutSwitchingMaterialRankTwoTensor` switches between two material properties of type `RankTwoTensor` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/XFEMCutSwitchingMaterialRankTwoTensor

## XFEMCutSwitchingMaterialRankThreeTensor

`XFEMCutSwitchingMaterialRankThreeTensor` switches between two material properties of type `RankThreeTensor` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/XFEMCutSwitchingMaterialRankThreeTensor

## XFEMCutSwitchingMaterialRankFourTensor

`XFEMCutSwitchingMaterialRankFourTensor` switches between two material properties of type `RankFourTensor` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/XFEMCutSwitchingMaterialRankFourTensor
