# GeometricCutSwitchingMaterial

!syntax description /Materials/GeometricCutSwitchingMaterialReal

To allow using XFEM to model interfaces between materials, the XFEM system uses geometric cut subdomain IDs to denote the subset of a standard MOOSE subdomain (element block) that a material point belongs to. Based on these geometric cut subdomains, `GeometricCutSwitchingMaterial` is a templated class that is used to switch between material properties depending on which geometric cut subdomain a point is located within. This class reads in multiple versions of a material property, all with different values of `base_name`, and sets the value of the material property without that base name to the name of the property that applies to the current geometric cut subdomain.

This class has four instantiations described below that are used to switch between properties with specific types:

## GeometricCutSwitchingMaterialReal

`GeometricCutSwitchingMaterialReal` switches between two material properties of type `Real` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/GeometricCutSwitchingMaterialReal

!syntax inputs /Materials/GeometricCutSwitchingMaterialReal

## GeometricCutSwitchingMaterialRankTwoTensor

`GeometricCutSwitchingMaterialRankTwoTensor` switches between two material properties of type `RankTwoTensor` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/GeometricCutSwitchingMaterialRankTwoTensor

## GeometricCutSwitchingMaterialRankThreeTensor

`GeometricCutSwitchingMaterialRankThreeTensor` switches between two material properties of type `RankThreeTensor` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/GeometricCutSwitchingMaterialRankThreeTensor

## GeometricCutSwitchingMaterialRankFourTensor

`GeometricCutSwitchingMaterialRankFourTensor` switches between two material properties of type `RankFourTensor` that coexist on the same element with different base_names.

## Example Input File Syntax

!syntax parameters /Materials/GeometricCutSwitchingMaterialRankFourTensor
