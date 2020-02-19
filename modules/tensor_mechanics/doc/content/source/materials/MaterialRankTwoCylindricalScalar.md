#Material Rank Two Cylindrical Scalar

!syntax description /Materials/MaterialRankTwoCylindricalScalar

## Description

Material Rank Two Cylindrical Scalar is used within
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md) for problems
with a Cylindrical coordinate system. This class provides methods to calculate
several different scalar stress ($\boldsymbol{\sigma}$) and strain
($\boldsymbol{\epsilon}$)quantities for a Rank-2 tensor, as described in
[RankTwoScalarTools](RankTwoScalarTools.md).  

In some types of calculations, the scalar quantity is calculated in a
user-specified direction; in other calculation types the user can specify the
start and end points of a line along which the  scalar quantity is calculated.
This value is stored as a material property, allowing calculations at quadrature
points,  which is more precise than the values returned by the similar class
[RankTwoScalarAux](RankTwoScalarAux.md)

If desired, `MaterialRankTwoCylindricalScalar` can be restricted to calculate the
scalar quantity data for a Rank-2 tensor at a single specified quadrature point
per element. This option is generally used only for debugging purposes.

!syntax parameters /Materials/MaterialRankTwoCylindricalScalar

!syntax inputs /Materials/MaterialRankTwoCylindricalScalar

!syntax children /Materials/MaterialRankTwoCylindricalScalar
