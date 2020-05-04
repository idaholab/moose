# Rank Two Cylindrical Component

!syntax description /Materials/RankTwoCylindricalComponent

## Description

This is a Material model used to extract components of a rank-2 tensor in a
cylindrical coordinate system. This can be used regardless of the coordinate
system used by the model.

This Material model is set up by
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md) automatically
when stress components are requested in the generate_output parameter, but can
also be set up directly by the user.  This class provides the ability to compute
hoop, radial, and axial stress ($\boldsymbol{\sigma}$) and strain
($\boldsymbol{\epsilon}$)quantities for a Rank-2 tensor, as described in
[RankTwoScalarTools](RankTwoScalarTools.md).  

The component of the rank-2 tensor extracted is stored as a scalar material
property, which allows for it to be more accurately represented in calculations
that use this quantity at quadrature points than would be possible using the
related [RankTwoScalarAux](RankTwoScalarAux.md)

!syntax parameters /Materials/RankTwoCylindricalComponent

!syntax inputs /Materials/RankTwoCylindricalComponent

!syntax children /Materials/RankTwoCylindricalComponent
