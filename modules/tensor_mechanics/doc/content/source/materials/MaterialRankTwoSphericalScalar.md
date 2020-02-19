#Material Rank Two Spherical Scalar

!syntax description /Materials/MaterialRankTwoSphericalScalar

## Description

Material Rank Two Spherical Scalar is used within
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md) for problems
with a Spherical coordinate system. This class provides methods to calculate
several different scalar stress ($\boldsymbol{\sigma}$) and strain
($\boldsymbol{\epsilon}$)quantities for a Rank-2 tensor, as described in
[RankTwoScalarTools](RankTwoScalarTools.md).  

In some types of calculations, the scalar quantity is calculated in a
user-specified direction; in other calculation types the user can specify the
end point with respect to the origin (0,0,0) of a line along which the  scalar
quantity is calculated. This value is stored as a material property, allowing
calculations at quadrature points,  which is more precise than the values
returned by the similar class [RankTwoScalarAux](RankTwoScalarAux.md)

If desired, `MaterialRankTwoSphericalScalar` can be restricted to calculate the
scalar quantity data for a Rank-2 tensor at a single specified quadrature point
per element. This option is generally used only for debugging purposes.

!syntax parameters /Materials/MaterialRankTwoSphericalScalar

!syntax inputs /Materials/MaterialRankTwoSphericalScalar

!syntax children /Materials/MaterialRankTwoSphericalScalar
