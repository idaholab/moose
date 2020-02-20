# Rank Two Directional Component

!syntax description /Materials/RankTwoDirectionalComponent

## Description

This is a Material model used to extract components of a rank-2 tensor in a
Cartesian coordinate system based on an input direction. This can be used
regardless of the coordinate system used by the model.

This Material model is used by
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md)
automatically, but can also be called directly by the user. This class
calculates the scalar value of a Rank-2 tensor, $T$, in the direction selected
by the user as shown by, as described in
[RankTwoScalarTools](RankTwoScalarTools.md).  


The component of the rank-2 tensor extracted is stored as a scalar material
property, which allows for it to be more accurately represented in calculations
that use this quantity at quadrature points than would be possible using the
related [RankTwoScalarAux](RankTwoScalarAux.md)

!syntax parameters /Materials/RankTwoDirectionalComponent

!syntax inputs /Materials/RankTwoDirectionalComponent

!syntax children /Materials/RankTwoDirectionalComponent
