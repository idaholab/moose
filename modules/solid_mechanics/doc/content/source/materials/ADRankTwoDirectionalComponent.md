# ADRank Two Directional Component

!syntax description /Materials/ADRankTwoDirectionalComponent

## Description

This is a ADMaterial model used to extract components of a rank-2 tensor in a
Cartesian coordinate system based on an input direction. This can be used
regardless of the coordinate system used by the model.

This ADMaterial model is set up by
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md) automatically
when stress components are requested in the generate_output parameter, but can
also be set up directly by the user. This class calculates the component of a
Rank-2 tensor, $T$, in the direction selected by the user as shown by, as
described in [RankTwoScalarTools](RankTwoScalarTools.md).  


The component of the rank-2 tensor extracted is stored as a scalar material
property, which allows for it to be more accurately represented in calculations
that use this quantity at quadrature points than would be possible using the
related [RankTwoScalarAux](RankTwoScalarAux.md)

!syntax parameters /Materials/ADRankTwoDirectionalComponent

!syntax inputs /Materials/ADRankTwoDirectionalComponent

!syntax children /Materials/ADRankTwoDirectionalComponent
