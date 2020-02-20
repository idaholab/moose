# Rank Two Invariant

!syntax description /Materials/RankTwoInvariant

## Description

This is a Material model used to extract an invariant of a rank-2 tensor in a
Cartesian coordinate system. This can be used regardless of the coordinate
system used by the model.

This Material model is used by
[TensorMechanics/Master](/Modules/TensorMechanics/Master/index.md)
automatically, but can also be called directly by the user. This class provides
the ability to compute  Max, Mid, and Min Primary stresses ($\boldsymbol{\sigma}$)
and strains ($\boldsymbol{\epsilon}$)quantities for a Rank-2 tensor, as
described in [RankTwoScalarTools](RankTwoScalarTools.md).  


The component of the rank-2 tensor extracted is stored as a scalar material
property, which allows for it to be more accurately represented in calculations
that use this quantity at quadrature points than would be possible using the
related [RankTwoScalarAux](RankTwoScalarAux.md)

!syntax parameters /Materials/RankTwoInvariant

!syntax inputs /Materials/RankTwoInvariant

!syntax children /Materials/RankTwoInvariant
